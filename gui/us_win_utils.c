#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>

char* GetLastError( void )
{
   return "";
}
int GetErrorCode( void )
{
   return 0;
}
int ShowInterpNames( char* inames, int mxchar )
{
   int stat = ( mxchar >= 0 ) ? 0 : 1;
   inames[0] = '\0';
   return stat;
}
int SendCommand( unsigned long int iid, char* interp, char* command )
{
   int stat = ( iid > 0 && interp != NULL && command != NULL ) ? 0 : 1;
   return stat;
}
int SendQuery( unsigned long int iid, char* interp, char* command, char* retval, int mxch )
{
   int stat = ( iid > 0 && interp != NULL && command != NULL ) ? 0 : 1;
   stat     = ( retval != NULL  &&  mxch > 0 ) ? stat : 2;
   return stat;
}



