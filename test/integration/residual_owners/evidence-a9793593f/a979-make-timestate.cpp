//! \file a979-make-timestate.cpp
// Prepare a local-only TimeState fixture from the already verified raw data.
#include "us_astfem_math.h"
#include "us_dataIO.h"
#include "us_simparms.h"
#include <QCoreApplication>
#include <QDebug>
int main( int argc, char** argv )
{
   QCoreApplication app( argc, argv );
   if ( argc != 3 )
   {
      return 2;
   }
   US_DataIO::RawData raw{};
   int                status = US_DataIO::readRawData( QString::fromLocal8Bit( argv[1] ), raw );
   if ( status != US_DataIO::OK )
   {
      return 3;
   }
   US_SimulationParameters params;
   US_SimulationParameters::computeSpeedSteps( &raw.scanData, params.speed_step );
   int records = US_AstfemMath::writetimestate( QString::fromLocal8Bit( argv[2] ), params, raw );
   qInfo() << "raw scans" << raw.scanCount() << "speed steps" << params.speed_step.size() << "TimeState records" << records;
   return records > 0 ? 0 : 4;
}
