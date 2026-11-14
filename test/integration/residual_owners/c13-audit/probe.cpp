//! \file probe.cpp
// C13 production reader initialization probe; no production repair.
#include "us_dataIO.h"
#include "us_util.h"
#include <QCoreApplication>
#include <QTemporaryDir>
#include <cstdint>
#include <cstdio>
#include <cstring>
int main( int argc, char** argv )
{
   QCoreApplication   app( argc, argv );
   QTemporaryDir      dir;
   US_DataIO::RawData input{};
   memcpy( input.type, "RA", 2 );
   memcpy( input.rawGUID, "fedcba9876543210", 16 );
   input.cell        = 1;
   input.channel     = 'A';
   input.description = "C13 deterministic reader probe";
   input.xvalues     = { 5.8, 5.81, 5.82 };
   US_DataIO::Scan scan{};
   scan.temperature  = 20;
   scan.rpm          = 45000;
   scan.seconds      = 300;
   scan.omega2t      = 2.5e9;
   scan.wavelength   = 280;
   scan.delta_r      = .01;
   scan.plateau      = 0.25;
   scan.rvalues      = { .2, .25, .3 };
   scan.interpolated = QByteArray( 1, '\0' );
   input.scanData.append( scan );
   QString rawName = dir.filePath( "audit.RA.1.A.280.auc" );
   if ( US_DataIO::writeRawData( rawName, input ) != US_DataIO::OK )
   {
      return 2;
   }
   US_DataIO::RawData loaded{};
   if ( US_DataIO::readRawData( rawName, loaded ) != US_DataIO::OK )
   {
      return 3;
   }
   uint64_t bits = 0;
   memcpy( &bits, &loaded.scanData[0].plateau, sizeof bits );
   printf( "raw plateau bits=%016llx\n", (unsigned long long) bits );
   printf( "raw values=%g,%g,%g\n", loaded.scanData[0].rvalues[0], loaded.scanData[0].rvalues[1], loaded.scanData[0].rvalues[2] );
   US_DataIO::EditValues ev;
   ev.expType       = "Velocity";
   ev.runID         = "audit";
   ev.cell          = "1";
   ev.channel       = "A";
   ev.wavelength    = "280";
   ev.dataGUID      = US_Util::uuid_unparse( (uchar*) input.rawGUID );
   ev.editGUID      = "00000000-0000-0000-0000-000000000013";
   ev.rangeLeft     = 5.8;
   ev.rangeRight    = 5.82;
   ev.plateau       = 5.81;
   ev.meniscus      = 5.7;
   ev.bottom        = 7.2;
   QString editName = "audit.2601010000.RA.1.A.280.xml";
   if ( US_DataIO::writeEdits( dir.filePath( editName ), ev ) != US_DataIO::OK )
   {
      return 4;
   }
   QVector<US_DataIO::EditedData> edited;
   QVector<US_DataIO::RawData>    raw;
   if ( US_DataIO::loadData( dir.path(), editName, edited, raw ) != US_DataIO::OK )
   {
      return 5;
   }
   printf( "edited plateau=%g floating=%d radius=%g values=%g,%g,%g\n",
           edited[0].scanData[0].plateau,
           int( edited[0].floatingData ),
           edited[0].plateau,
           edited[0].scanData[0].rvalues[0],
           edited[0].scanData[0].rvalues[1],
           edited[0].scanData[0].rvalues[2] );
   return 0;
}
