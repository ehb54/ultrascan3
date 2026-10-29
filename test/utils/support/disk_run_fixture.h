//! \file disk_run_fixture.h
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

#ifndef DISK_RUN_FIXTURE_H
#define DISK_RUN_FIXTURE_H

#include "us_convert.h"
#include "us_experiment.h"
#include "us_simparms.h"
#include "us_util.h"

#include <QString>
#include <QVector>

namespace us3test
{

//! One cell/channel/wavelength dataset of a run.
struct DatasetSpec
{
      int     cell       = 1;
      char    channel    = 'A';
      QString wavelength = "280"; //!< the filename field, verbatim
      int     scans      = 3;
      int     points     = 8;
};

//! What writeDiskRun() put on disk, for a test to assert against.
struct DiskRun
{
      QString     runID;
      QString     runType;
      QString     dir;         //!< absolute, with a trailing separator
      QStringList aucFiles;    //!< basenames, in the order they were written
      QStringList tripleDescs; //!< "cell / channel / wavelength", same order
      QStringList rawGUIDs;    //!< same order; also each dataset's triple GUID
      QString     solutionGUID;
      QString     projectGUID;
};

//! The triple description US_Convert::readUS3Disk() derives from a filename.
QString tripleDescFor( const QString& runType, const DatasetSpec& spec );

//! Write one run archive under parentDir and describe what landed there.
//!
//! The directory is named for the run, which is what the reader requires: it
//! takes the run ID from the last path component.
DiskRun writeDiskRun( const QString&                  parentDir,
                      const QString&                  runID,
                      const QString&                  runType,
                      const QVector<DatasetSpec>&     datasets,
                      const QVector<SP_SPEEDPROFILE>& speedSteps = QVector<SP_SPEEDPROFILE>() );
}

#endif // DISK_RUN_FIXTURE_H
