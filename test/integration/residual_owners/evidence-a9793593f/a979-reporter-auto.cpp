//! \file a979-reporter-auto.cpp
// Actual AUTO reporter entry point for the isolated test workflow.
#include "us_license.h"
#include "us_license_t.h"
#include "us_reporter_gmp.h"
#include <QTimer>
int main( int argc, char** argv )
{
   QApplication application( argc, argv );
#include "main1.inc"
   US_ReporterGMP w( QString( "AUTO" ) );
   w.show();
   QTimer::singleShot( 100,
                       &w,
                       [&w]
                       {
                          QMap<QString, QString> p;
                          p["invID_passed"]   = "1";
                          p["aprofileguid"]   = "cc48212a-6274-53da-ab79-008670962462";
                          p["protocolName"]   = "Residual owner verification TEST ONLY";
                          p["runID"]          = "1011";
                          p["experimentName"] = "multicell-demo-ra";
                          p["filename"]       = "multicell-demo-ra";
                          p["autoflowID"]     = "1240001";
                          p["analysisIDs"]    = "1240001";
                          p["statusID"]       = "0";
                          p["dataSource"]     = "dataDiskAUC:Absorbance";
                          p["expType"]        = "VELOCITY";
                          w.loadRun_auto( p );
                       } );
   return application.exec();
}
