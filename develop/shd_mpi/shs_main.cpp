#include "shs.h"

int main( int argc, char **argv )
{
   SHS shs;
   shs.build( 50, .1, 1000 );
   shs.write_c_table( string( "shs_data.h" ) );
   exit( 0 );
}
