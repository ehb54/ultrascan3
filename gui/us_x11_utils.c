/*
 * This set of utility routines handles IPC on X11 systems.
 * They are based on the x11win.c source in RasMol.
 *
 * The primary public functions it provides are:
 *
 *   ShowInterpNames( ... )   - get the list of interpreters
 *   SendCommand( ... )       - send a command to an interpreter
 *   SendQuery( ... )         - send a command and get the response
 *   GetLastError( ... )      - get last IPC error text
 *   GetErrorCode( ... )      - get last IPC error code
 *
 */
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>

#define UnusedArgument(a) ((a) = (a))
#define _LEN_ERRBUF 1024
#define _LEN_ERRMSG _LEN_ERRBUF - 1

/* Return a pointer to the error buffer */
char *ErrorBuffer() {
   static char errbuf[ _LEN_ERRBUF ];
   return ( char * ) errbuf;
}

/* Handle any IPC error */
int HandleIPCError(Display *dpy, XErrorEvent *ptr) {
   char ecd = ptr->error_code;
   char *ebuf = ErrorBuffer();
   char *emsg = ebuf + 1;
   ebuf[ 0 ] = ecd;
   int errcode = ( int ) ecd;

   XGetErrorText(dpy, errcode, emsg, _LEN_ERRMSG);
   if (errcode != 0)
      fprintf(stderr, "HIE: errcode %d errtext %s\n", errcode, emsg);
   return 0;
}

/* Return the last IPC error text */
char *GetLastError() {
   return ErrorBuffer() + 1;
}

/* Return the last IPC error code */
int GetErrorCode() {
   return ( int ) ErrorBuffer()[ 0 ];
}

/* Clear the error buffer of the last error */
void ClearLastError() {
   char *eb = ErrorBuffer();
   eb[ 0 ] = '\0';
   eb[ 1 ] = '\0';
}

/* Public function to return interpreter names */
int ShowInterpNames(char *inames, int mxchar) {
   Display *dpy;
   Atom InterpAtom;
   Atom type;
   int (*handler)();
   int result, format, nchar;
   long unsigned int len, left;
   unsigned char *registry;

   dpy = XOpenDisplay(NULL);

   if (dpy == NULL)
      return 0;

   InterpAtom = XInternAtom(dpy, "InterpRegistry", False);
   handler = XSetErrorHandler(HandleIPCError);

   result = XGetWindowProperty(
      dpy, RootWindow(dpy, 0), InterpAtom, 0L, 300L, False, XA_STRING, &type, &format, &len, &left, &registry);

   XSync(dpy, False);
   XSetErrorHandler(handler);

   if (result == 0) {
      nchar = ( int ) len;
      nchar = nchar < mxchar ? nchar : mxchar;
      memcpy(inames, ( char * ) registry, nchar);
   }

   else {
      sprintf(inames, "InterpNamesERROR=%d", result);
      nchar = ( int ) strlen(inames) + 1;
   }

   return nchar;
}

#ifndef MAC
/* Return an integer from a digit character */
int digittoint(char dchr) {
   int intout = 0;

   if (dchr >= 'a' && dchr <= 'f')
      intout = ( int ) (dchr - 'a') + 10;
   else if (dchr >= 'A' && dchr <= 'F')
      intout = ( int ) (dchr - 'A') + 10;
   else if (dchr >= '0' && dchr <= '9')
      intout = ( int ) (dchr - '0');

   return intout;
}
#endif

/* Get the interpreter ID associated with a given interpreter name */
int CheckInterpName(char *name, unsigned long *interpid) {
   unsigned char *registry;
   unsigned long len, left;
   int format;
   Atom type;
   Atom InterpAtom;
   Display *dpy;

   register int result;
   register char *ptr;
   register int (*handler)();

   dpy = XOpenDisplay(NULL);
   InterpAtom = XInternAtom(dpy, "InterpRegistry", False);
   registry = NULL;
   *interpid = 0;
   handler = XSetErrorHandler(HandleIPCError);
   result = XGetWindowProperty(
      dpy, RootWindow(dpy, 0), InterpAtom, 0, 100000, False, XA_STRING, &type, &format, &len, &left, &registry);
   fprintf(stderr, "CK: result %d Success %d\n", result, Success);
   fprintf(stderr, "CK: format %d type %d %d\n", format, ( int ) type, ( int ) XA_STRING);
   XSync(dpy, False);
   XSetErrorHandler(handler);

   if ((result != Success) || (format != 8) || (type != XA_STRING)) {
      if ((type != None) && registry)
         XFree(( char * ) registry);
      return (False);
   }

   ptr = ( char * ) registry;
   fprintf(stderr, "CK0: ptr %s\n", ptr);
   while (*ptr) {
      *interpid = 0;
      while (*ptr != ' ') {
         if (!*ptr)
            break;
         if (isxdigit(*ptr)) {
            *interpid = ((*interpid) << 4) | ( unsigned long ) digittoint(*ptr);
         }
         else {
            break;
         }
         ptr++;
      }
      fprintf(stderr, "CK1: name %s interpid %ld\n", name, *interpid);

      /* Strip leading blanks */

      while (*ptr == ' ')
         ptr++;

      /* Compare Interp Name */
      if (!strcmp(ptr, name)) {
         XFree(( char * ) registry);
         fprintf(stderr, "CK2: name %s ptr %s\n", name, ptr);
         return True;
      }

      while (*ptr++)
         ;
   }
   fprintf(stderr, "CK: name %s interpid %ld\n", name, *interpid);

   XFree(( char * ) registry);
   return (False);
}

/* Send an IPC command to a specified interp (no expectation of response) */
int SendCommand(long unsigned int interpid, char *interp, char *command) {
   static int Serial = 0;
   Display *dpy;
   Atom CommAtom;
   Window commWindow;
   int (*handler)();
   int len, result, ii, jj;
   char buffer[ 256 ];
   char *errmsg;
   Window MainWin;

   dpy = XOpenDisplay(NULL);

   if (dpy == NULL)
      return 0;

   MainWin = RootWindow(dpy, 0);
   CommAtom = XInternAtom(dpy, "Comm", False);

   if (interpid == 0L)
      CheckInterpName(interp, &interpid);
   fprintf(stderr, "SC: interpid %ld %lx\n", interpid, interpid);
   commWindow = ( Window ) interpid;

   handler = XSetErrorHandler(HandleIPCError);

   buffer[ 0 ] = '\0';
   buffer[ 1 ] = 'c';
   buffer[ 2 ] = '\0';
   buffer[ 3 ] = '-';
   buffer[ 4 ] = 'n';
   buffer[ 5 ] = ' ';
   jj = 6;
   len = strlen(interp);
   for (ii = 0; ii < len; ii++)
      buffer[ jj++ ] = interp[ ii ];
   buffer[ jj++ ] = '\0';
   sprintf(buffer + jj, "-r %x %d", ( int ) MainWin, ++Serial);
   jj += strlen(buffer + jj);
   buffer[ jj++ ] = '\0';
   buffer[ jj++ ] = '-';
   buffer[ jj++ ] = 's';
   buffer[ jj++ ] = ' ';
   len = strlen(command);
   for (ii = 0; ii < len; ii++)
      buffer[ jj++ ] = command[ ii ];
   buffer[ jj++ ] = '\0';
   len = jj;
   ClearLastError();

   XChangeProperty(dpy, commWindow, CommAtom, XA_STRING, 8, PropModeAppend, ( unsigned char * ) buffer, len);

   XSync(dpy, False);
   result = GetErrorCode();
   errmsg = GetLastError();
   fprintf(stderr, "SC: Buff+3 %s  len %d result %d %s\n", buffer + 3, len, result, errmsg);
   //   XSync( dpy, False );
   XSetErrorHandler(handler);

   return result;
}

/* Send a "Query" ("Command" with expectation of a response) */
int SendQuery(long unsigned int interpid, char *interp, char *command, char *response, int maxrch) {
   static unsigned long len, left;
   static unsigned char *propstr;

   Display *dpy;
   Atom type;
   Atom CommAtom;
   int result, xlen, kchr, format;
   char buffer[ 256 ];
   char *ptr;
   char *ptr_last;
   char *xp;
   char *serialstr;
   char *codestr;
   char *resultstr;
   char *errInfostr;
   char *errCodestr;
   Window MainWin;

   /* First send the command */
   result = SendCommand(interpid, interp, command);

   if (result != 0) { /* If the send had an error, return an error message */
      sprintf(response, "%d : %s", result, GetLastError());
      return result;
   }

   /* Then read the response */
   dpy = XOpenDisplay(NULL);

   if (dpy == NULL)
      return 0;

   MainWin = RootWindow(dpy, 0);
   CommAtom = XInternAtom(dpy, "Comm", False);
   propstr = NULL;

   if (interpid == 0L)
      CheckInterpName(interp, &interpid);
   fprintf(stderr, "SQ: interpid %ld %lx\n", interpid, interpid);

   result = XGetWindowProperty(dpy, MainWin, CommAtom, 0, 1024, True, XA_STRING, &type, &format, &len, &left, &propstr);

   fprintf(stderr, "SQ: len %ld propstr %lx\n", len, ( ulong ) propstr);
   ptr = ( char * ) propstr; /* Working pointer to response */
   ptr_last = ptr + len; /* Pointer to end of response */
   if (ptr != NULL)
      fprintf(
         stderr, "SQ: r0-8 %x %x %x %x %x %x %x %x\n", ptr[ 0 ], ptr[ 1 ], ptr[ 2 ], ptr[ 3 ], ptr[ 4 ], ptr[ 5 ],
         ptr[ 6 ], ptr[ 7 ]);
   fprintf(
      stderr, "SQ: r0-8 %c %c %c %c %c %c %c %c\n", ptr[ 0 ], ptr[ 1 ], ptr[ 2 ], ptr[ 3 ], ptr[ 4 ], ptr[ 5 ],
      ptr[ 6 ], ptr[ 7 ]);
   kchr = maxrch; /* Count of output character room left */
   serialstr = NULL;
   codestr = NULL;
   resultstr = NULL;
   errInfostr = NULL;
   errCodestr = NULL;

   if ((result != Success) || (type != XA_STRING) || (format != 8)) { /* Return immediately if error or unexpected type/format */
      if (command != NULL)
         XFree(ptr);
      return result;
   }

   /* Parse the response */
   if (ptr[ 0 ] == '\0') { /* Response is Tcl/Tk4 type */
      ptr++;

      while (ptr < ptr_last) { /* Walk through response characters */

         if (ptr[ 0 ] == 'r' && ptr[ 1 ] == '\0') { // Handle a response string
            ptr += 2;
            while ((ptr < ptr_last) && (ptr[ 0 ] == '-')) {
               fprintf(stderr, "SQ: ptr=%s\n", ptr);
               if (ptr[ 1 ] == 's' && ptr[ 2 ] == ' ') {
                  serialstr = ptr + 3;
               }
               else if (ptr[ 1 ] == 'c' && ptr[ 2 ] == ' ') {
                  codestr = ptr + 3;
               }
               else if (ptr[ 1 ] == 'r' && ptr[ 2 ] == ' ') {
                  resultstr = ptr + 3;
               }
               else if (ptr[ 1 ] == 'i' && ptr[ 2 ] == ' ') {
                  errInfostr = ptr + 3;
               }
               else if (ptr[ 1 ] == 'e' && ptr[ 2 ] == ' ') {
                  errCodestr = ptr + 3;
               }

               while (ptr[ 0 ] != '\0')
                  ptr++;
               ptr++;
               fprintf(stderr, "SQ: (2)ptr=%s p0 p1 %c %c\n", ptr, ptr[ 0 ], ptr[ 1 ]);
            }

            xp = response;
            strcpy(xp, "Interp response:");
            kchr -= strlen(xp);
            xlen = strlen(xp);
            kchr -= xlen;
            xp += xlen;

            if (serialstr != NULL && strlen(serialstr) > 0) {
               if (kchr > 20) {
                  strncat(xp, " serial: ", 9);
                  strncat(xp + 8, serialstr, 10);
                  xlen = strlen(xp);
                  kchr -= xlen;
                  xp += xlen;
               }
               else {
                  strcpy(buffer, " serial: ");
                  strcat(buffer, serialstr);
                  strncat(xp, buffer, kchr - 1);
                  xp[ kchr - 1 ] = '\0';
                  break;
               }
            }

            if (codestr != NULL && strlen(codestr) > 0) {
               if (kchr > 18) {
                  strncat(xp, " code: ", 7);
                  strncat(xp + 8, codestr, 10);
                  xlen = strlen(xp);
                  kchr -= xlen;
                  xp += xlen;
               }
               else {
                  strcpy(buffer, " code: ");
                  strcat(buffer, codestr);
                  strncat(xp, buffer, kchr - 1);
                  xp[ kchr - 1 ] = '\0';
                  break;
               }
            }

            if (resultstr != NULL && strlen(resultstr) > 0) {
               if (kchr > 70) {
                  strncat(xp, " result: ", 9);
                  strncat(xp + 8, resultstr, 60);
                  xlen = strlen(xp);
                  kchr -= xlen;
                  xp += xlen;
               }
               else {
                  strcpy(buffer, " result: ");
                  strcat(buffer, resultstr);
                  strncat(xp, buffer, kchr - 1);
                  xp[ kchr - 1 ] = '\0';
                  break;
               }
            }

            if (errInfostr != NULL && strlen(errInfostr) > 0) {
               if (kchr > 73) {
                  strncat(xp, " errorInfo: ", 12);
                  strncat(xp + 8, errInfostr, 60);
                  xlen = strlen(xp);
                  kchr -= xlen;
                  xp += xlen;
               }
               else {
                  strcpy(buffer, " errorInfo: ");
                  strcat(buffer, errInfostr);
                  strncat(xp, buffer, kchr - 1);
                  xp[ kchr - 1 ] = '\0';
                  break;
               }
            }

            if (errCodestr != NULL && strlen(errCodestr) > 0) {
               if (kchr > 73) {
                  strncat(xp, " errorCode: ", 12);
                  strncat(xp + 8, errCodestr, 60);
                  xlen = strlen(xp);
                  kchr -= xlen;
                  xp += xlen;
               }
               else {
                  strcpy(buffer, " errorCode: ");
                  strcat(buffer, errCodestr);
                  strncat(xp, buffer, kchr - 1);
                  xp[ kchr - 1 ] = '\0';
                  break;
               }
            }
         } /* End response string */

         else { /* Skip anything not a response */
            while (ptr[ 0 ] != '\0')
               ptr++;
            ptr++;
         }
         fprintf(stderr, "SQ: (3)ptr=%s p0 p1 %c %c\n", ptr, ptr[ 0 ], ptr[ 1 ]);
         fprintf(stderr, "SQ: (3)ptr=%lx ptrl=%lx\n", ( ulong ) ptr, ( ulong ) ptr_last);

      } /* End response characters loop */
   } /* End Tcl/Tk4.0 type response */

   XFree(propstr);

   return result;
}
