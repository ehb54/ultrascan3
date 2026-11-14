//! \file constructor_probe.cpp
#include "us_widgets.h"
#include <QApplication>
#include <cstdio>

int main( int argc, char** argv )
{
   QApplication app( argc, argv );
   US_Widgets   owner;
   const double values[] = { 0.0, -1.0, 0.001, 90.0 };
   for ( double value : values )
   {
      QwtCounter* counter = owner.us_counter( 3, -1000.0, 1000.0, value );
      if ( counter->minimumWidth() <= 0 || counter->minimumWidth() > 2000 )
      {
         return 1;
      }
   }
   std::puts( "PASS: real counters have bounded widths for zero, negative, fractional and positive values" );
   return 0;
}
