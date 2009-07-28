#include "us_crc.h"

unsigned long US_Crc::crc32( 
      unsigned long crc, const unsigned char* buf, unsigned int len )
{
   if ( buf == 0 ) return 0UL;

   crc = crc ^ 0xffffffffUL;
   
   if ( len ) do 
   {
     crc = crc_table[ ( (int)crc ^ ( *buf++ ) ) & 0xff ] ^ ( crc >> 8 );
   } while ( --len );
               
   return crc ^ 0xffffffffUL;
}
