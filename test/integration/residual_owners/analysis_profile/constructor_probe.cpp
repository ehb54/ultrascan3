//! \file analysis-profile-probe.cpp
#include "us_analysis_profile.h"
#include <QApplication>
#include <cstdio>

int main( int argc, char** argv )
{
   QApplication          app( argc, argv );
   US_AnalysisProfileGui profile;
   auto*                 panel = profile.findChild<US_AnaprofPan2DSA*>();
   if ( panel == nullptr || panel->getIValue( "changed" ) != 0 )
   {
      return 1;
   }
   std::puts( "PASS: real analysis profile constructs with an unchanged 2DSA panel" );
   return 0;
}
