//! \file test_us_null_db.cpp
// Every US_Experiment and US_ConvertIO entry point that takes a connection
// refuses a null one instead of dereferencing it.

#include <gtest/gtest.h>

#include "ius_db2.h"
#include "us_convert.h"
#include "us_convertio.h"
#include "us_experiment.h"

#include <QList>
#include <QString>
#include <QVector>

namespace
{
class NullDb : public ::testing::Test
{
   protected:
      US_Experiment                 experiment;
      QList<US_Convert::TripleInfo> triples;
      QVector<SP_SPEEDPROFILE>      speedsteps;
      QString                       dir = "/nonexistent/";
      const QString                 refused( const char* what ) const
      {
         return QObject::tr( IUS_DB2::notConnectedMessage ).arg( what );
      }
};

TEST_F( NullDb, ExperimentRefusesNullConnection )
{
   EXPECT_EQ( experiment.checkRunID( nullptr ), IUS_DB2::NOT_CONNECTED );
   EXPECT_EQ( experiment.checkRunID_auto( 1, nullptr ), IUS_DB2::NOT_CONNECTED );
   EXPECT_EQ( experiment.saveToDB( false, nullptr, speedsteps ), IUS_DB2::NOT_CONNECTED );
   EXPECT_EQ( experiment.saveToDB_auto( false, nullptr, speedsteps, 1 ), IUS_DB2::NOT_CONNECTED );
   EXPECT_EQ( experiment.readFromDB( "run", nullptr, speedsteps ), IUS_DB2::NOT_CONNECTED );
   // Without a connection this works on local files, so a null one is not an error.
   EXPECT_TRUE( US_Experiment::deleteRunPcsaMrecs( nullptr, "1", "run" ) );
}

TEST_F( NullDb, ConvertIORefusesNullConnection )
{
   EXPECT_EQ( US_ConvertIO::writeRawDataToDB( experiment, triples, dir, nullptr ), refused( "writeRawDataToDB" ) );
   EXPECT_EQ( US_ConvertIO::readDBExperiment( "run", dir, nullptr, speedsteps ), refused( "readDBExperiment" ) );
   EXPECT_EQ( US_ConvertIO::checkDiskData( experiment, triples, nullptr ), IUS_DB2::NOT_CONNECTED );
   EXPECT_EQ( US_ConvertIO::checkDiskData_auto( experiment, triples, nullptr ), IUS_DB2::NOT_CONNECTED );
}
} // namespace
