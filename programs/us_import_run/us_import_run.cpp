//! \file us_import_run.cpp
//! \brief Load one AUC run from a US3 run directory into a LIMS database.
//!
//! The run is read and written with the desktop import's own code:
//! US_ConvertIO::readDiskRun() to read it, then US_Experiment::saveToDB(),
//! US_ConvertIO::writeRawDataToDB() and the edited-data calls to write it, all
//! in one transaction that is rolled back on any failure.
//!
//! It checks more strictly than the desktop import.  It loads the LIMS test
//! corpus, simulated runs that are loaded into a scratch database and dumped
//! as seed data, so a run has to load exactly as generated.  Anything the
//! desktop would repair or skip is refused instead.
//!
//! Exit status: 0 loaded, or validated with --dry-run; 1 bad arguments or the
//! records could not be staged; 2 the run failed validation; 3 no investigator
//! could be resolved or a database step failed.

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDir>
#include <QRegularExpression>
#include <QSet>
#include <QTemporaryDir>
#include <QTextStream>

#include "us_convert.h"
#include "us_convertio.h"
#include "us_dataIO.h"
#include "us_db2.h"
#include "us_defines.h"
#include "us_experiment.h"
#include "us_settings.h"
#include "us_simparms.h"
#include "us_solution.h"
#include "us_util.h"

namespace
{

//! Standard output.
QTextStream& out()
{
   static QTextStream s( stdout );
   return s;
}
//! Standard error.
QTextStream& err()
{
   static QTextStream s( stderr );
   return s;
}

//! solution.description is VARCHAR(80) in the LIMS schema.  Reject a
//! run before opening a transaction rather than letting new_solution()
//! fail with LAST_INSERT_ID() == 0 and a misleading missing-solution error.
constexpr int      MAX_SOLUTION_DESCRIPTION_CHARACTERS = 80;

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
   static const QRegularExpression re( "^(.+)\\.([^.]+)\\.([A-Z]{2})\\.(\\d+)\\.([A-Z])\\.([^.]+)\\.xml$" );
   return re;
}

//! Point US_Settings at \p root, so a load neither reads nor writes the
//! user's own settings.  Call before anything reads a setting: the store is
//! chosen once per process.
void isolateSettings( const QString& root )
{
   qputenv( "US3_SETTINGS_ROOT", root.toUtf8() );
}

//! Parse a personID option.  False unless \p text is a positive integer.
bool parsePersonId( const QString& text, int& id )
{
   bool ok = false;
   id      = text.toInt( &ok );
   return ok && id > 0;
}

//! A chemistry record's filename prefix and staging directory.
struct RecordKind
{
      QChar   letter;
      QString directory;
};

//! The record kinds staged from the run directory.
const QList<RecordKind>& recordKinds()
{
   // P records join S/B/A because the experiment XML names a project GUID and
   // US_Project::readFromDisk() resolves it out of the same data tree.
   static const QList<RecordKind> kinds{ { 'P', "projects" }, { 'S', "solutions" }, { 'B', "buffers" }, { 'A', "analytes" } };
   return kinds;
}

//! \brief Copy the run directory's P/S/B/A records into the data tree that
//!        US_Project and US_Solution read them from.
//!
//! The data tree is in the private staging directory main() creates, so it
//! starts empty.  The copies are renumbered from 1, because a record is found
//! by the GUID inside it, not by its name.
//! \param staged Set to the number of records copied, per staging directory.
bool stageRecords( const QString& run_dir, QMap<QString, int>& staged, QString& error )
{
   QDir source( run_dir );

   for ( const RecordKind& kind : recordKinds() )
   {
      const QString pattern     = QString( kind.letter ) + "???????.xml";
      const QString target_path = US_Settings::dataDir() + "/" + kind.directory;

      QDir          target;
      if ( !target.mkpath( target_path ) )
      {
         error = "Could not create the staging directory " + target_path;
         return false;
      }

      int count = 0;
      for ( const QString& name : source.entryList( QStringList( pattern ), QDir::Files, QDir::Name ) )
      {
         const QString staged_name = QString( "%1%2.xml" ).arg( kind.letter ).arg( count + 1, 7, 10, QChar( '0' ) );

         if ( !QFile::copy( source.filePath( name ), target_path + "/" + staged_name ) )
         {
            error = "Could not stage " + kind.directory + " file " + name;
            return false;
         }
         count++;
      }

      staged[kind.directory] = count;
   }

   return true;
}

//! Refuse a directory whose experiment records name a run other than this one.
bool rejectExtraRuns( const QString& run_dir, const QString& run_id, QString& error )
{
   // US_Convert::readUS3Disk() takes the first .auc file's run ID for the whole
   // directory, and readDiskRun() names the mixture when the .auc files
   // disagree.  Neither looks at the experiment records, so a directory holding
   // one run's data beside another run's metadata still reads as one run.
   // This program loads exactly one run, so the directory may hold nothing else.
   QStringList others;

   for ( const QString& name : QDir( run_dir ).entryList( QStringList( "*.xml" ), QDir::Files, QDir::Name ) )
   {
      const QRegularExpressionMatch m = experimentFilePattern().match( name );

      if ( m.hasMatch() && m.captured( 1 ) != run_id && !others.contains( m.captured( 1 ) ) )
      {
         others << m.captured( 1 );
      }
   }

   if ( others.isEmpty() )
   {
      return true;
   }

   error = "Directory " + run_dir + " holds run " + run_id + " alongside experiment records for " + others.join( ", " ) +
           "\nThis loads one run; split the directory by run ID first";
   return false;
}

//! One of the run directory's edit files, paired with the raw dataset it names.
struct EditFile
{
      QString path;     //!< Full path, for the blob upload
      QString name;     //!< Basename, stored as editedData.filename
      QString editGUID; //!< Identity of the edit itself
      QString dataGUID; //!< The rawDataGUID it applies to
};

//! Find the run's edit files and check that each one matches a raw dataset
//! and that every raw dataset has exactly one.
bool collectEdits( const US_ConvertIO::DiskRun& run, QList<EditFile>& edits, QString& error )
{
   const QString                      run_id   = run.runID;
   const QVector<US_DataIO::RawData>& raw_data = run.rawData;

   QDir                               dir( run.directory );
   edits.clear();

   // Every check below reads the triple that sits at a raw dataset's index, so
   // a run whose two lists have drifted apart cannot be validated at all.
   if ( run.triples.size() != raw_data.size() )
   {
      error = "Run " + run_id + " has " + QString::number( raw_data.size() ) + " data files but " +
              QString::number( run.triples.size() ) + " triples";
      return false;
   }

   QMap<QString, int> raw_by_guid;
   for ( int raw_index = 0; raw_index < raw_data.size(); raw_index++ )
   {
      const US_DataIO::RawData& raw  = raw_data[raw_index];
      const QString             guid = US_Util::uuid_unparse( reinterpret_cast<const unsigned char*>( raw.rawGUID ) ).toLower();

      if ( raw_by_guid.contains( guid ) )
      {
         error = "Run " + run_id + " contains duplicate raw data GUID " + guid;
         return false;
      }

      raw_by_guid.insert( guid, raw_index );
   }

   QSet<QString> covered;
   QSet<QString> edit_guids;

   for ( const QString& name : dir.entryList( QStringList( "*.xml" ), QDir::Files, QDir::Name ) )
   {
      const QRegularExpressionMatch match = editFilePattern().match( name );
      if ( !match.hasMatch() )
      {
         continue;
      }

      // Ignore edits owned by another run in the directory.
      if ( match.captured( 1 ) != run_id )
      {
         continue;
      }

      EditFile edit;
      edit.path = dir.filePath( name );
      edit.name = name;

      US_DataIO::EditValues values;
      const int             read_status = US_DataIO::readEdits( edit.path, values );
      if ( read_status != US_DataIO::OK )
      {
         error = "Could not read the edit file " + name + " (status " + QString::number( read_status ) + ")";
         return false;
      }

      edit.editGUID = values.editGUID;
      edit.dataGUID = values.dataGUID;

      if ( edit.dataGUID.isEmpty() || edit.editGUID.isEmpty() )
      {
         error = "Edit file " + name + " names no rawDataGUID or editGUID";
         return false;
      }

      const QString data_guid = edit.dataGUID.toLower();
      const QString edit_guid = edit.editGUID.toLower();

      if ( values.runID != run_id )
      {
         error = "Edit file " + name + " identifies run " + values.runID + ", not " + run_id;
         return false;
      }

      if ( match.captured( 4 ) != values.cell || match.captured( 5 ) != values.channel || match.captured( 6 ) != values.wavelength )
      {
         error = "Edit file " + name + " does not match the dataset triple in its XML";
         return false;
      }

      if ( !raw_by_guid.contains( data_guid ) )
      {
         error = "Edit file " + name + " points to raw data " + edit.dataGUID + ", which is not present in run " + run_id;
         return false;
      }

      const int                 raw_index = raw_by_guid[data_guid];
      const US_DataIO::RawData& raw       = raw_data[raw_index];
      const QString             raw_type  = QString::fromLatin1( raw.type, 2 );

      if ( match.captured( 3 ) != raw_type || values.cell.toInt() != raw.cell || values.channel != QString( raw.channel ) )
      {
         error = "Edit file " + name + " does not match the type, cell, or channel of raw data " + edit.dataGUID;
         return false;
      }

      // US_Edit names an edit file from its .auc filename, copying the
      // wavelength field as written, so that field is what the edit name has
      // to match.  Neither the triple nor the scan holds that text:
      // readUS3Disk() divides the field by 1000 for a WA run, where it is a
      // radius, and us_astfem_sim writes the sentinel 123 into it when the
      // scan wavelength is below 99.
      const QString auc_wavelength = run.triples[raw_index].tripleFilename.section( ".", -2, -2 );

      if ( values.wavelength != auc_wavelength )
      {
         error = "Edit file " + name + " is for wavelength " + values.wavelength + ", but raw data " + edit.dataGUID + " is " +
                 auc_wavelength;
         return false;
      }

      if ( edit_guids.contains( edit_guid ) )
      {
         error = "Run " + run_id + " contains duplicate edit GUID " + edit.editGUID;
         return false;
      }

      if ( covered.contains( data_guid ) )
      {
         error = "Run " + run_id + " contains more than one edit for raw data " + edit.dataGUID;
         return false;
      }

      edit_guids.insert( edit_guid );
      covered.insert( data_guid );
      edits << edit;
   }

   // LIMS cannot submit a dataset for analysis without its edit.
   QStringList missing;
   for ( auto it = raw_by_guid.cbegin(); it != raw_by_guid.cend(); ++it )
   {
      if ( !covered.contains( it.key() ) )
      {
         missing << it.key();
      }
   }

   if ( !missing.isEmpty() )
   {
      error = "No edit file for raw data " + missing.join( ", " ) +
              "\nEvery .auc needs its edit XML, or the run cannot be "
              "submitted for analysis";
      return false;
   }

   return true;
}

//! Create an editedData row and upload its blob for each edit, stopping at the
//! first failure.
bool uploadEdits( const QList<EditFile>& edits, const QString& run_id, US_DB2* db, QString& error )
{
   // Create only: a run is loaded into a fresh database once, so an edit that
   // already has a record means the run is not what it claims to be, and
   // nothing is updated.
   for ( const EditFile& edit : edits )
   {
      int rawDataID = -1;

      if ( US_ConvertIO::findRawDataId( db, edit.dataGUID, rawDataID, error ) != US_DB2::OK )
      {
         error = "No raw data record for GUID " + edit.dataGUID + ", named by edit file " + edit.name + " (" + error + ")";
         return false;
      }

      US_ConvertIO::EditedDataRecord record;
      record.rawDataID = rawDataID;
      record.editGUID  = edit.editGUID;
      record.label     = run_id;
      record.filename  = edit.name;

      int edit_id = -1;

      if ( US_ConvertIO::createEditedData( db, record, edit_id, error ) != US_DB2::OK )
      {
         return false;
      }

      if ( US_ConvertIO::uploadEditedDataBlob( db, edit_id, edit.path, error ) != US_DB2::OK )
      {
         return false;
      }
   }

   return true;
}

//! One run, read and checked but not yet written.
struct PreparedRun
{
      US_ConvertIO::DiskRun disk;  //!< Raw data, triples, experiment, speed steps
      QList<EditFile>       edits; //!< Found by collectEdits(); readDiskRun() does not read edits
};

//! Read \p run_dir and validate it, loading its project and solutions from the
//! staged records.  Touches no database.
bool prepareRun( const QString& run_dir, const QString& wanted_run, const QString& wanted_type, PreparedRun& run, QString& error )
{
   // Reading is shared with the desktop; the strictness is not.  readDiskRun()
   // reports what it finds and leaves the verdict to its caller, and here every
   // one of those reports is fatal: a run that goes into a test database has
   // to be exactly what it claims to be.
   const int status = US_ConvertIO::readDiskRun( run_dir, run.disk, error );

   if ( status != US_Convert::OK )
   {
      return false;
   }

   const QString run_id = run.disk.runID;

   if ( !wanted_run.isEmpty() && wanted_run != run_id )
   {
      error = "--run-id " + wanted_run + " does not match " + run_dir + ", which holds run " + run_id;
      return false;
   }

   if ( !rejectExtraRuns( run.disk.directory, run_id, error ) )
   {
      return false;
   }

   if ( !wanted_type.isEmpty() && wanted_type != run.disk.runType )
   {
      error = "--run-type " + wanted_type + " does not match run " + run_id + ", whose AUC files are " + run.disk.runType;
      return false;
   }

   // The desktop import reports a missing project and clears it.  This refuses
   // instead: a run loaded without its project has a new one created at write
   // time, so the seed would differ on every build.
   US_Project& project = run.disk.experiment.project;

   if ( project.projectGUID.isEmpty() )
   {
      error = "Run " + run_id + " names no project GUID";
      return false;
   }

   if ( project.readFromDisk( project.projectGUID ) != US_DB2::OK )
   {
      error = "No project record for GUID " + project.projectGUID + " (run " + run_id +
              ")\nThe run directory must include the P record the "
              "simulation wrote";
      return false;
   }

   // The desktop import recovers from a missing solution; this refuses one.
   for ( int ii = 0; ii < run.disk.triples.size(); ii++ )
   {
      US_Solution& solution = run.disk.triples[ii].solution;
      QString      guid     = solution.solutionGUID;

      if ( guid.isEmpty() )
      {
         error = "Triple " + run.disk.triples[ii].tripleDesc + " of run " + run_id + " names no solution GUID";
         return false;
      }

      if ( solution.readFromDisk( guid ) != US_DB2::OK )
      {
         error = "No solution record for GUID " + guid + " (triple " + run.disk.triples[ii].tripleDesc + " of run " + run_id +
                 ")\nThe run directory must include the solution XML the simulation "
                 "wrote";
         return false;
      }

      const int description_length = solution.solutionDesc.toUcs4().size();
      if ( description_length > MAX_SOLUTION_DESCRIPTION_CHARACTERS )
      {
         error = "Solution " + guid + " in run " + run_id + " (triple " + run.disk.triples[ii].tripleDesc +
                 ") has a description of " + QString::number( description_length ) +
                 " characters; the "
                 "LIMS solution.description limit is " +
                 QString::number( MAX_SOLUTION_DESCRIPTION_CHARACTERS );
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
      explicit TransactionGuard( US_DB2& connection ) : db( connection )
      {
      }

      ~TransactionGuard()
      {
         if ( !armed )
         {
            return;
         }

         QString error;

         if ( !db.rollbackTransaction( error ) )
         {
            err() << "Rollback also failed: " << error << Qt::endl;
         }
      }

      //! Called once the commit has succeeded.
      void disarm()
      {
         armed = false;
      }

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
   parser.setApplicationDescription( "Load a generated AUC run into a US3 LIMS database using the desktop's "
                                     "own write path." );
   const QCommandLineOption help_option    = parser.addHelpOption();
   const QCommandLineOption version_option = parser.addVersionOption();

   QCommandLineOption       dir_option( "dir",
                                  "Directory holding exactly one run -- its .auc files, edit XML and "
                                        "experiment XML -- along with the P/S/B/A records they name. The "
                                        "directory name is the run ID",
                                  "path" );
   QCommandLineOption       host_option( "host", "Database host, optionally host:port (default localhost)", "host", "localhost" );
   QCommandLineOption       db_option( "database",
                                 "Database name. Required unless --dry-run is given, and refused unless it is one of " +
                                    allowedDatabases().join( ", " ) + " or --force is given",
                                 "name" );
   QCommandLineOption       user_option( "user", "Database user", "user", "us3" );
   QCommandLineOption       password_option( "password",
                                       "Database password (default: $US3_DB_PASSWORD). Prefer the environment "
                                             "variable: a command-line value is visible to other users",
                                       "password" );
   QCommandLineOption       investigator_option( "investigator-id",
                                           "personID recorded as the owner of everything created. Defaults to the "
                                                 "person the US3 credentials authenticate as. Another person's ID is "
                                                 "accepted, so the rows can be owned by someone other than the account "
                                                 "that wrote them",
                                           "id" );
   QCommandLineOption       investigator_guid_option( "investigator-guid",
                                                "personGUID of the US3 account to act as. Defaults to the investigator "
                                                      "GUID in the experiment XML",
                                                "guid" );
   QCommandLineOption       investigator_password_option( "investigator-password",
                                                    "Password of that US3 account (default: $US3_INV_PASSWORD). This is "
                                                          "the US3 login, not the MySQL password given by --password",
                                                    "password" );
   QCommandLineOption       operator_option( "operator-id",
                                       "personID recorded as the instrument operator, who must hold a permit "
                                             "for the instrument. Defaults to the operator in the experiment XML, or "
                                             "to the investigator when it names none",
                                       "id" );
   QCommandLineOption       work_dir_option( "work-dir",
                                       "Directory to create the temporary staging directory in, which holds "
                                             "the copied records and isolated settings and is removed on exit "
                                             "(default: the system temporary directory)",
                                       "path" );
   QCommandLineOption       run_id_option( "run-id",
                                     "Refuse the load unless --dir holds this run. Selects nothing: the "
                                           "directory already names exactly one run",
                                     "id" );
   QCommandLineOption       run_type_option( "run-type",
                                       "Two-character optical type. Defaults to the one named by the "
                                             "experiment XML",
                                       "type" );
   QCommandLineOption       force_option( "force",
                                    "Accept a database name outside the allowed list. This is the only "
                                          "thing it does: it does not permit overwriting a run that is already "
                                          "loaded, which fails either way" );
   QCommandLineOption       dry_run_option( "dry-run",
                                      "Read and validate the run directory, then stop before connecting to "
                                            "the database" );

   parser.addOptions( { dir_option,
                        host_option,
                        db_option,
                        user_option,
                        password_option,
                        investigator_option,
                        investigator_guid_option,
                        investigator_password_option,
                        operator_option,
                        work_dir_option,
                        run_id_option,
                        run_type_option,
                        force_option,
                        dry_run_option } );

   if ( !parser.parse( QCoreApplication::arguments() ) )
   {
      err() << parser.errorText() << Qt::endl;
      return 1;
   }
   if ( parser.isSet( version_option ) )
   {
      out() << QCoreApplication::applicationName() << " " << QCoreApplication::applicationVersion() << Qt::endl;
      return 0;
   }
   if ( parser.isSet( help_option ) )
   {
      out() << parser.helpText() << Qt::endl;
      return 0;
   }

   if ( !parser.isSet( dir_option ) )
   {
      err() << "--dir is required" << Qt::endl;
      return 1;
   }

   // A dry run stops before connecting, so it needs no database.
   const bool    dry_run  = parser.isSet( dry_run_option );
   const QString database = parser.value( db_option );
   if ( !dry_run && database.isEmpty() )
   {
      err() << "--database is required; there is no default" << Qt::endl;
      return 1;
   }
   if ( !dry_run && !allowedDatabases().contains( database ) && !parser.isSet( force_option ) )
   {
      err() << "Refusing to write to \"" << database << "\": expected one of " << allowedDatabases().join( ", " )
            << "\nPass --force to accept this name anyway." << Qt::endl;
      return 1;
   }

   int investigator_id = 0;
   if ( parser.isSet( investigator_option ) && !parsePersonId( parser.value( investigator_option ), investigator_id ) )
   {
      err() << "--investigator-id must be a positive integer, not \"" << parser.value( investigator_option ) << "\"" << Qt::endl;
      return 1;
   }

   int operator_id = 0;
   if ( parser.isSet( operator_option ) && !parsePersonId( parser.value( operator_option ), operator_id ) )
   {
      err() << "--operator-id must be a positive integer, not \"" << parser.value( operator_option ) << "\"" << Qt::endl;
      return 1;
   }

   QDir source( parser.value( dir_option ) );
   if ( !source.exists() )
   {
      err() << "No such directory: " << source.path() << Qt::endl;
      return 1;
   }
   const QString run_dir = source.absolutePath();

   // Records are staged into a directory created here and removed on exit, so
   // no existing directory, and none of the user's own records, is touched.
   const QString work_parent =
      parser.isSet( work_dir_option ) ? QDir( parser.value( work_dir_option ) ).absolutePath() : QDir::tempPath();
   if ( !QDir().mkpath( work_parent ) )
   {
      err() << "Could not create " << work_parent << Qt::endl;
      return 1;
   }

   QTemporaryDir work( work_parent + "/us_import_run-XXXXXX" );
   if ( !work.isValid() )
   {
      err() << "Could not create a staging directory in " << work_parent << ": " << work.errorString() << Qt::endl;
      return 1;
   }
   const QString work_dir = work.path();

   isolateSettings( work_dir + "/settings" );
   US_Settings::set_workBaseDir( work_dir );

   QMap<QString, int> staged;
   QString            stage_error;
   if ( !stageRecords( run_dir, staged, stage_error ) )
   {
      err() << stage_error << Qt::endl;
      return 1;
   }

   out() << "run dir      : " << run_dir << Qt::endl;
   out() << "work dir     : " << work_dir << Qt::endl;
   out() << "staged       : " << staged["projects"] << " projects, " << staged["solutions"] << " solutions, " << staged["buffers"]
         << " buffers, " << staged["analytes"] << " analytes" << Qt::endl;
   out().flush();

   PreparedRun run;
   QString     prepare_error;

   if ( !prepareRun( run_dir, parser.value( run_id_option ), parser.value( run_type_option ).toUpper(), run, prepare_error ) )
   {
      err() << prepare_error << Qt::endl;
      return 2;
   }

   out() << "run          : " << run.disk.runID << " : " << run.disk.runType << ", " << run.disk.triples.size() << " triples, "
         << run.edits.size() << " edits" << Qt::endl;

   if ( dry_run )
   {
      out() << "dry run, nothing written to the database" << Qt::endl;
      return 0;
   }

   QString investigator_guid = parser.value( investigator_guid_option );
   if ( investigator_guid.isEmpty() )
   {
      investigator_guid = run.disk.experiment.invGUID;
   }

   if ( investigator_guid.isEmpty() )
   {
      err() << "No investigator: the experiment XML names no GUID and "
               "--investigator-guid was not given"
            << Qt::endl;
      return 3;
   }

   // connectAuthenticated() takes the credentials as arguments, so no database
   // profile is written into the user's settings.
   // The environment keeps the passwords out of the process list; an option
   // given on the command line still wins.
   const QString db_password =
      parser.isSet( password_option ) ? parser.value( password_option ) : qEnvironmentVariable( "US3_DB_PASSWORD" );
   const QString inv_password = parser.isSet( investigator_password_option ) ? parser.value( investigator_password_option )
                                                                             : qEnvironmentVariable( "US3_INV_PASSWORD" );

   US_DB2        db;
   QString       db_error;
   if ( !db.connectAuthenticated( parser.value( host_option ),
                                  database,
                                  parser.value( user_option ),
                                  db_password,
                                  investigator_guid,
                                  inv_password,
                                  db_error ) )
   {
      err() << "Could not connect to " << database << " as " << investigator_guid << ": " << db_error << Qt::endl;
      return 3;
   }

   // validate_user leaves the authenticated person ID in this session value.
   int investigator = 0;
   db.rawQuery( "SELECT @US3_ID" );
   if ( db.next() )
   {
      investigator = db.value( 0 ).toInt();
   }

   if ( investigator_id > 0 )
   {
      investigator = investigator_id;
   }

   if ( investigator <= 0 )
   {
      err() << "Authenticated, but no personID came back for " << investigator_guid << Qt::endl;
      return 3;
   }

   out() << "investigator : " << investigator << " (" << investigator_guid << ")" << Qt::endl;

   QString transaction_error;
   if ( !db.beginTransaction( transaction_error ) )
   {
      err() << "Could not begin database transaction: " << transaction_error << Qt::endl;
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
   if ( operator_id > 0 )
   {
      run.disk.experiment.operatorID = operator_id;
   }
   else if ( run.disk.experiment.operatorID <= 0 )
   {
      run.disk.experiment.operatorID = investigator;
   }

   // Create only: in a fresh database a run ID that is already present is a
   // duplicate load, and saveToDB() returns DUPFIELD rather than rewriting the
   // run.  saveToDB() writes the speed-step rows from the vector it is given,
   // so this passes the steps the experiment XML carried.
   const int save_status = run.disk.experiment.saveToDB( false, &db, run.disk.speedSteps );

   if ( save_status == US_DB2::DUPFIELD )
   {
      err() << "Run " << run.disk.runID << " is already in " << database << Qt::endl;
      return 3;
   }

   if ( save_status != US_DB2::OK )
   {
      err() << "Could not save experiment " << run.disk.runID << " (status " << save_status << "): " << db.lastError() << Qt::endl;
      return 3;
   }

   // saveToDB() does not report a failed speed-step insert, so count the rows
   // it left rather than commit a run without its speed profile.
   QVector<SP_SPEEDPROFILE> stored_steps;
   const int                stored = US_SimulationParameters::speedstepsFromDB( &db, run.disk.experiment.expID, stored_steps );

   if ( stored != run.disk.speedSteps.size() )
   {
      err() << "Experiment " << run.disk.runID << " has " << stored << " speed steps in the database, not "
            << run.disk.speedSteps.size() << ": " << db.lastError() << Qt::endl;
      return 3;
   }

   const QString write_error = US_ConvertIO::writeRawDataToDB( run.disk.experiment, run.disk.triples, run.disk.directory, &db );

   if ( !write_error.isEmpty() )
   {
      err() << write_error << Qt::endl;
      return 3;
   }

   QString edit_error;
   if ( !uploadEdits( run.edits, run.disk.runID, &db, edit_error ) )
   {
      err() << edit_error << Qt::endl;
      return 3;
   }

   if ( !db.commitTransaction( transaction_error ) )
   {
      err() << "Could not commit database transaction: " << transaction_error << Qt::endl;
      return 3;
   }

   transaction.disarm();

   out() << "loaded       : " << run.disk.runID << ", experimentID " << run.disk.experiment.expID << ", " << run.disk.triples.size()
         << " triples, " << run.edits.size() << " edits into " << database << Qt::endl;
   return 0;
}
