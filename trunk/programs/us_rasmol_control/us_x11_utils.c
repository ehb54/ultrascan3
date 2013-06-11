#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>

#define UnusedArgument(a) ((a)=(a))

int HandleIPCError( Display *dpy, XErrorEvent *ptr )
{
   UnusedArgument( dpy );
   UnusedArgument( ptr );
   return 0;
}

int ShowInterpNames( char* inames, int mxchar )
{
   Display* dpy;
   Atom     InterpAtom;
   Atom     type;
   int      (*handler)();
   int      result, format, nchar;
   long unsigned int len, left;
   unsigned char*    registry;

   dpy = XOpenDisplay( NULL );

   if ( dpy == NULL )   return 0;

   InterpAtom = XInternAtom( dpy, "InterpRegistry", False );
   handler    = XSetErrorHandler( HandleIPCError );

   result     = XGetWindowProperty( dpy, RootWindow( dpy, 0 ), InterpAtom,
         0L, 300L, False, XA_STRING, &type, &format, &len, &left,
         &registry );

   XSync( dpy, False );
   XSetErrorHandler( handler );

   if ( result == 0 )
   {
      nchar      = (int)len;
      nchar      = nchar < mxchar ? nchar : mxchar;
      memcpy( inames, (char*)registry, nchar );
   }

   else
   {
      sprintf( inames, "InterpNamesERROR=%d", result );
      nchar      = (int)strlen( inames ) + 1;
   }

   return nchar;
}

#ifndef MAC
int digittoint( char dchr )
{
   int intout = 0;

   if      ( dchr >= 'a'  &&  dchr <= 'f' )
      intout  = (int)( dchr - 'a' ) + 10;
   else if ( dchr >= 'A'  &&  dchr <= 'F' )
      intout  = (int)( dchr - 'A' ) + 10;
   else if ( dchr >= '0'  &&  dchr <= '9' )
      intout  = (int)( dchr - '0' );

   return intout;
}
#endif

int CheckInterpName( char *name, unsigned long *interpid )
{
   unsigned char *registry;
   unsigned long len,left;
   int format;
   Atom type;
   Atom     InterpAtom;
   Display* dpy;

   register int result;
   register char *ptr;
   register int (*handler)();

   dpy        = XOpenDisplay( NULL );
   InterpAtom = XInternAtom( dpy, "InterpRegistry", False );
   registry   = NULL;
   *interpid  = 0;
   handler    = XSetErrorHandler( HandleIPCError );
   result     = XGetWindowProperty( dpy, RootWindow(dpy,0), InterpAtom,
                                    0, 100000, False, XA_STRING, &type,
                                    &format, &len, &left, &registry );
fprintf(stderr,"CK: result %d Success %d\n",result,Success);
fprintf(stderr,"CK: format %d type %d %d\n",format,(int)type,(int)XA_STRING);
   XSync(dpy,False);
   XSetErrorHandler(handler);

   if( (result!=Success) || (format!=8) || (type!=XA_STRING) )
   {
      if( (type!=None) && registry ) XFree( (char*)registry );
      return( False );
   }

   ptr = (char*)registry;
fprintf(stderr,"CK0: ptr %s\n",ptr);
   while( *ptr )
   {
      *interpid = 0;
      while( *ptr != ' ' )
      {
         if (!*ptr ) break;
         if (isxdigit(*ptr))
         {
            *interpid = ((*interpid)<<4)|(unsigned long)digittoint(*ptr);
         }
         else
         {
            break;
         }
         ptr++;
      }
fprintf(stderr,"CK1: name %s interpid %ld\n",name,*interpid);

      /* Strip leading blanks */

      while (*ptr == ' ') ptr++;

      /* Compare Interp Name */
      if ( !strcmp(ptr,name) )
      {
         XFree( (char*)registry );
fprintf(stderr,"CK2: name %s ptr %s\n",name,ptr);
         return True;
      }

      while( *ptr++ );
   }
fprintf(stderr,"CK: name %s interpid %ld\n",name,*interpid);

   XFree( (char*)registry );
   return( False );
}

int SendCommand( char* interp, char* command )
{
   static int Serial = 0;
   Display* dpy;
   Atom     CommAtom;
   Window   commWindow;
   int      (*handler)();
   int len;
   long unsigned int interpid;
   char buffer[256];
   Window   MainWin;

   dpy        = XOpenDisplay( NULL );

   if ( dpy == NULL )   return 0;

   MainWin    = RootWindow( dpy, 0 );
   CommAtom   = XInternAtom( dpy, "Comm", False );

   CheckInterpName( interp, &interpid );
fprintf(stderr,"SC: interpid %ld\n",interpid);
   commWindow = (Window)interpid;

   handler    = XSetErrorHandler( HandleIPCError );

   buffer[0]  = '\0';
   buffer[1]  = 'c';
   buffer[2]  = '\0';
   buffer[3]  = '-';
   buffer[4]  = 'n';
   buffer[5]  = ' ';
   int jj     = 6;
   int ii;
   len        = strlen( interp );
   for ( ii = 0; ii < len; ii++ )
      buffer[ jj++ ] = interp[ ii ];
   buffer[ jj++ ] = '\0';
   sprintf( buffer+jj, "-r %x %d", (int)MainWin, ++Serial );
   jj        += strlen( buffer+jj );
   buffer[ jj++ ] = '\0';
   buffer[ jj++ ] = '-';
   buffer[ jj++ ] = 's';
   buffer[ jj++ ] = ' ';
   len        = strlen( command );
   for ( ii = 0; ii < len; ii++ )
      buffer[ jj++ ] = command[ ii ];
   buffer[ jj++ ]  = '\0';
   len        = jj;

   XChangeProperty( dpy, commWindow, CommAtom, XA_STRING, 8,
                    PropModeAppend, (unsigned char*)buffer, len );

fprintf(stderr,"SC: Buff+3 %s  len %d",buffer+3,len);
   XSync( dpy, False );
   XSetErrorHandler( handler );

   return len;
}

