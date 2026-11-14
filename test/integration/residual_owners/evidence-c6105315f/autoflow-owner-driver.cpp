//! \file autoflow-owner-driver.cpp
// Test entry point: invokes the actual owner's public initialization and overlay slot.
#include "us_autoflow_analysis.h"
#include "us_license.h"
#include "us_license_t.h"
#include <QTimer>
int main( int argc, char** argv )
{
   QApplication application( argc, argv );
#include "main1.inc"
   US_Analysis_auto w;
   w.show();
   QTimer::singleShot( 100,
                       &w,
                       [&w]
                       {
                          QMap<QString, QString> p;
                          p["invID_passed"] = "1";
                          p["protocolName"] = "Residual owner verification TEST ONLY";
                          p["aprofileguid"] = "cc48212a-6274-53da-ab79-008670962462";
                          p["filename"]     = "multicell-demo-ra";
                          p["analysisIDs"]  = "1240001";
                          p["autoflowID"]   = "1240001";
                          p["statusID"]     = "0";
                          p["expType"]      = "VELOCITY";
                          p["dataSource"]   = "dataDiskAUC:Absorbance";
                          w.initPanel( p );
                          QMetaObject::invokeMethod(
                             &w, "show_overlay", Qt::DirectConnection, Q_ARG( QString, QString( "2DSA-IT (1 / A / 123)" ) ) );
                       } );
   return application.exec();
}
