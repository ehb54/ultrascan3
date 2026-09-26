// A US3 run archive on disk, written through the production serializers.
//
// Phase 1 characterizes the disk-reading prefix of US_ConvertGui::loadUS3Disk()
// before that code moves.  Those tests have to start from archives the shipping
// writers produced, not from hand-rolled XML, or they would pin the fixture's
// idea of the format rather than UltraScan's.  So this helper only calls
// US_DataIO::writeRawData(), US_Experiment::saveToDisk() and
// US_DataIO::writeEdits(), and it never patches what they wrote.
//
// A negative test may copy a run this produced and corrupt it explicitly in its
// own temporary directory.  That is the only sanctioned mutation.

#pragma once

#include "us_convert.h"
#include "us_dataIO.h"
#include "us_experiment.h"
#include "us_simparms.h"
#include "us_util.h"

#include <QDir>
#include <QString>
#include <QUuid>
#include <QVector>

#include <cstring>

namespace us3test
{

//! One cell/channel/wavelength dataset of a run.
struct DatasetSpec
{
   int     cell       = 1;
   char    channel    = 'A';
   QString wavelength = "280";   //!< the filename field, verbatim
   int     scans      = 3;
   int     points     = 8;
};

//! What writeDiskRun() put on disk, for a test to assert against.
struct DiskRun
{
   QString     runID;
   QString     runType;
   QString     dir;          //!< absolute, with a trailing separator
   QStringList aucFiles;     //!< basenames, in the order they were written
   QStringList tripleDescs;  //!< "cell / channel / wavelength", same order
   QStringList rawGUIDs;     //!< same order; also each dataset's triple GUID
   QString     solutionGUID;
   QString     projectGUID;
};

namespace detail
{

//! A stable GUID for a fixture, so a failure names the same identity twice.
inline QString fixtureGuid( const QString& key )
{
   static const QUuid ns( QStringLiteral(
                             "{6f3f9a02-2c0e-4a5b-9a2a-6f0d1c5b7e41}" ) );

   return QUuid::createUuidV5( ns, key.toUtf8() ).toString().mid( 1, 36 );
}

inline US_DataIO::Scan buildScan( int points, double base, double seconds )
{
   US_DataIO::Scan scan;
   scan.temperature = 20.0;
   scan.rpm         = 45000.0;
   scan.seconds     = seconds;
   scan.omega2t     = 2.5e10 + seconds;
   scan.wavelength  = 280.0;
   scan.delta_r     = 0.01;
   scan.nz_stddev   = false;

   for ( int point = 0; point < points; point++ )
   {
      scan.rvalues << base + 0.05 * point;
      scan.stddevs << 0.0;
   }

   // One bit per reading, rounded up to whole bytes; the writer copies exactly
   // that many without checking, so it has to be sized correctly here.
   scan.interpolated = QByteArray( ( points + 7 ) / 8, '\0' );

   return scan;
}

inline US_DataIO::RawData buildRawData( const QString& runType,
                                        const DatasetSpec& spec,
                                        const QString& guid )
{
   US_DataIO::RawData data;
   memcpy( data.type, runType.toLatin1().constData(), 2 );
   US_Util::uuid_parse( guid, (unsigned char*)data.rawGUID );

   data.cell        = spec.cell;
   data.channel     = spec.channel;
   data.description = "disk-run fixture";

   for ( int point = 0; point < spec.points; point++ )
      data.xvalues << 5.8 + 0.01 * point;

   for ( int scan = 0; scan < spec.scans; scan++ )
      data.scanData << buildScan( spec.points, 0.2 + scan,
                                  300.0 * ( scan + 1 ) );

   return data;
}
}

//! The triple description US_Convert::readUS3Disk() derives from a filename.
inline QString tripleDescFor( const QString& runType, const DatasetSpec& spec )
{
   const QString wavelength = ( runType == "WA" )
      ? QString::number( spec.wavelength.toDouble() / 1000.0 )
      : spec.wavelength;

   return QString::number( spec.cell ) + " / " + QString( spec.channel )
        + " / " + wavelength;
}

//! Write one run archive under parentDir and describe what landed there.
//!
//! The directory is named for the run, which is what the reader requires: it
//! takes the run ID from the last path component.
inline DiskRun writeDiskRun( const QString& parentDir,
                             const QString& runID,
                             const QString& runType,
                             const QVector< DatasetSpec >& datasets,
                             const QVector< SP_SPEEDPROFILE >& speedSteps
                                = QVector< SP_SPEEDPROFILE >() )
{
   DiskRun run;
   run.runID        = runID;
   run.runType      = runType;
   run.dir          = parentDir + "/" + runID + "/";
   run.solutionGUID = detail::fixtureGuid( runID + "/solution" );
   run.projectGUID  = detail::fixtureGuid( runID + "/project" );

   QDir().mkpath( run.dir );

   QList< US_Convert::TripleInfo > triples;

   for ( int ii = 0; ii < datasets.size(); ii++ )
   {
      const DatasetSpec& spec = datasets[ ii ];
      const QString guid = detail::fixtureGuid(
         runID + "/raw/" + QString::number( ii ) );

      const QString name = runID + "." + runType + "."
                         + QString::number( spec.cell ) + "."
                         + QString( spec.channel ) + "."
                         + spec.wavelength + ".auc";

      US_DataIO::RawData data = detail::buildRawData( runType, spec, guid );

      if ( US_DataIO::writeRawData( run.dir + name, data ) != US_DataIO::OK )
         return DiskRun();

      US_Convert::TripleInfo triple;
      triple.clear();
      triple.tripleID              = ii + 1;
      triple.tripleDesc            = tripleDescFor( runType, spec );
      triple.tripleFilename        = name;
      triple.excluded              = false;
      triple.centerpiece           = 1;
      // Not -1: US_Experiment::saveToDisk() starts its run-length encoding of
      // repeated solutions from that value, so a first triple carrying it is
      // written with an empty solution GUID.  See the characterization test
      // BlanksTheSolutionGuidOfADatasetWhoseSolutionIdIsUnset.
      triple.solution.solutionID   = 1;
      triple.solution.solutionGUID = run.solutionGUID;
      triple.solution.solutionDesc = "disk-run fixture solution";
      US_Util::uuid_parse( guid, (unsigned char*)triple.tripleGUID );

      triples << triple;

      run.aucFiles    << name;
      run.tripleDescs << triple.tripleDesc;
      run.rawGUIDs    << guid;
   }

   US_Experiment experiment;
   experiment.clear();
   experiment.expGUID             = detail::fixtureGuid( runID + "/experiment" );
   experiment.expType             = "velocity";
   experiment.runID               = runID;
   experiment.labID               = 1;
   experiment.instrumentID        = 1;
   experiment.rotorID             = 1;
   experiment.opticalSystem       = runType.toLatin1();
   experiment.runTemp             = "20";
   experiment.label               = runID;
   experiment.project.projectGUID = run.projectGUID;
   experiment.project.projectDesc = "disk-run fixture project";

   QVector< SP_SPEEDPROFILE > steps = speedSteps;

   if ( experiment.saveToDisk( triples, runType, runID, run.dir, steps )
        != US_Convert::OK )
      return DiskRun();

   return run;
}
}
