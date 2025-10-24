
#ifdef WIN32
# define MSCPLAT
# include <sys/timeb.h>
#else
# define AN6PLAT
# include <sys/time.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <math.h>
#include <string.h>

   // #define txtfile FILE*                /* The type of a text file */
   // #define binfile int                  /* The type of raw binary file handles */

#define CHARTYPE
#define DATETIME
#define MATHFUNC
#define PRIOFUNC
#define STRGFUNC
#define UBIOFUNC
#define PACKFUNC
#define RAWMFUNC
#define MEMOFUNC
   typedef short SHORT;

#define FNC_DECLARE

#define fifmalloc(x) malloc((int)(x))
#define fifmemc(x,y,z) memcmp(x,y,z)
#define fifcopy(x,y,z) memcpy(y,x,z)

   /*
     ;/hdr/ ************************************************************************
     ;
     ; platform.h: Define platforms Hosting Code
     ;
     ; useful notes and assumptions:
     ;
     ; This header file contains all the symbols and typedefs needed to work with the
     ; the information management system on a variety of platforms. The assumption
     ; in the design of this header is that each module written within the system
     ; includes this header using the notation:
     ;
     ;    #include <platform.h>
     ;
     ; Preceeding this include various symbols are defined specifying the particular
     ; types of functions needed by the module. Using this technique all platform
     ; dependent information can be inserted into this header file. The function
     ; codes never change when the code moves from one platform to another. In
     ; particular, platform specific "ifdefs" are never placed in the code.
     ; The particular symbols recognized by this header are as follows:
     ;
     ; Symbol     Description of use
     ; ------     ------------------
     ; CHARTYPE   Access to character type information
     ; DATETIME   Gives access to date and time functions
     ; ECVTFUNC   Gives access to E-format double conversion
     ; LONGMEMO   Gives access to long memory
     ; MATHFUNC   Gives access to mathematical functions
     ; MEMOFUNC   Gives access to memory allocation functions
     ; OSYSFUNC   Gives access to operating system
     ; PACKFUNC   Gives access fixed packing functions
     ; PRIOFUNC   Gives access to the portable I/O
     ; RAWMFUNC   Gives access to raw memory operations
     ; STRGFUNC   Gives access to string functions
     ; UBIOFUNC   Gives access to raw unbuffered I/O functions
     ; VARARGUS   Gives access to variable arguments
     ; SERVICES   Gives access to virtual service types
     ;
     ; When compiling a symbol identifying the target platform must be defined on
     ; the command for the C compiler being used. The platform identifiers
     ; recognized by this version are as follows:
     ;
     ;/hdr/ ************************************************************************
   */
#ifndef platform_h

#define platform_h

#define PRM_VERSION          "V9.96 (05/06/12)"
#define PRM_VNUMBER          996

#define USR_VERSION          "GM-105(BETA.001)"
#define USR_VNUMBER          105

#define REP_VOID             0
#define REP_VARCHAR          1
#define REP_CODE             2
#define REP_SHORT            3
#define REP_LONG             4
#define REP_SINGLE           5
#define REP_DOUBLE           6
#define REP_FSTRING          7
#define REP_DATE             8
#define REP_DATETIME         9
#define REP_SYMBOL           10
#define REP_INVERTED         11
#define REP_USERTYPE         12
#define REP_ENGINE           13

#ifdef MSCWIND               /* 32 bit MicroSoft C using windows */
#define MSCPLAT
#define USETXTFNC            /* Use Gui for text I/O */
#endif

#ifdef AIXTERM
#define AIXPLAT
#define USETXTFNC            /* Use Gui for text I/O */
#endif

#ifdef MSCPLAT               /* 32 bit MicroSoft C under WIN32S */
#ifndef _CRT_SECURE_NO_DEPRECATE 
#define _CRT_SECURE_NO_DEPRECATE  1
#endif
#define DLL_EXPORT __declspec(dllexport)
#define A86PLAT
#if !defined( MINGW )
#define longlong  _int64
#else
#define longlong  long long
#endif
#define longdouble long double
#define LL_C(x) x##L
#else
#define longlong long long
#define LL_C(x) x##LL
#define longdouble long double
#endif /* MSCPLAT */

#ifdef MPEPLAT
#define AN4PLAT
#endif

#ifdef ZORPLAT               /* Zortech C under DOS extented */
#define OSMSDOS              /* This platform operates under MS-DOS */
#define ANSPLAT              /* This platform supports ANSI C */
#define FPROTOTYPE           /* This platform uses function prototypes */
#define LITTLENDER   1       /* This platform does have little-ender sex */
#define CRUNTYPE     1       /* Runtime host C is ANSI4 */
#define NEEDSPACKF           /* This platform needs packing functions */
#define HASLNGFN             /* Uses long memory as explicit functions */
   typedef int LONG;            /* Signed 4-byte integers */
   typedef unsigned char UBYTE; /* Unsigned 1-byte integer */
   typedef unsigned int ULONG;  /* Unsigned 4-byte integers */
   typedef unsigned short USHORT;/* Unsigned 2-byte integers */
#endif /* ZORPLAT */

#ifdef UNXPLAT               /* A generic UNIX platform */
#define OSUNIX               /* This platform operates under UNIX */
#define NEEDMMOVE            /* This platform needs an overlaying move */
#define LITTLENDER   0       /* This platform does not have little-ender sex */
#define NEEDSPACKF           /* This platform needs packing functions */
#define VARUNIXV             /* Use Unix variable number of parameters */
#define CRUNTYPE     5       /* Runtime host C is KR4 */
#define LONG    int          /* Signed 4-byte integers */
#define UBYTE unsigned char  /* Unsigned 1-byte integer */
#define ULONG   unsigned int /* Unsigned 4-byte integers */
#define USHORT unsigned short  /* Unsigned 2-byte integers */
#endif /* UNXPLAT */

#ifdef AIXPLAT               /* An IBM Workstation with  AIX and xlc ANSI C */
#define OSUNIX               /* This platform operates under UNIX */
#define FPROTOTYPE           /* This platform uses function prototypes */
#define LITTLENDER   0       /* This platform does not have little-ender sex */
#define NEEDSPACKF           /* This platform needs packing functions */
#define ANSPLAT              /* This platform supports ANSI C */
#define CRUNTYPE      1      /* Runtime host C is ANSI4 */
   typedef int LONG;            /* Signed 4-byte integers */
   typedef unsigned char UBYTE; /* Unsigned 1-byte integer */
   typedef unsigned int ULONG;  /* Unsigned 4-byte integers */
   typedef unsigned short USHORT; /* Unsigned 2-byte integers */
#endif /* AIXPLAT */

#ifdef WTCPLAT               /* Watcom C under DOS extended */
#define A86PLAT
#endif

#ifdef BO2PLAT               /* Borland C under OS/2 */
#define A86PLAT
#endif

#ifdef A86PLAT               /* ANSI C on a DOS extended platform */
#define OSMSDOS              /* This platform operates under MS-DOS */
#define FPROTOTYPE           /* This platform uses function prototypes */
#define LITTLENDER   1       /* This platform does have little-ender sex */
#define ANSPLAT              /* This platform supports ANSI C */
#define CRUNTYPE     1       /* Runtime host C is ANSI4 */
#define LONG int             /* Signed 4-byte integers */
#define UBYTE unsigned char  /* Unsigned 1-byte integer */
#define ULONG unsigned int   /* Unsigned 4-byte integers */
#define USHORT unsigned short /* Unsigned 2-byte integers */
#endif /* A86PLAT */

#ifdef KRCPLAT               /* 32 bit MicroSoft C WIN32S with K+R */
#pragma warning(disable:4131)
#define OSMSDOS              /* This platform operates under MS-DOS */
#define LITTLENDER   1       /* This platform does have little-ender sex */
#define CRUNTYPE     5       /* Runtime host C is ANSI4 */
#define LONG int             /* Signed 4-byte integers */
#define UBYTE unsigned char  /* Unsigned 1-byte integer */
#define ULONG unsigned int   /* Unsigned 4-byte integers */
#define USHORT unsigned short  /* Unsigned 2-byte integers */
#define A86PLAT
#endif /* KRCPLAT */

#ifdef AZTPLAT               /* An Apple Macintosh using Aztec C */
#define OSMAC                /* This platform operates under the MacIntosh */
#define SHORTMEMO            /* This platform has a short memory */
#define LITTLENDER   0       /* This platform does not have little-ender sex */
#define NEEDSPACKF           /* This platform needs packing functions */
#define VARUNIXV             /* Use Unix variable number of parameters */
#define CRUNTYPE     5       /* Runtime host C is KR4 */
#define LONG    int          /* Signed 4-byte integers */
#define UBYTE unsigned char  /* Unsigned 1-byte integer */
#define ULONG   unsigned int /* Unsigned 4-byte integers */
#define USHORT  unsigned short /* Unsigned 2-byte integers */
#endif /* AZTPLAT */

#ifdef MPWPLAT               /* An Apple Macintosh the MPW shell with MPW C */
#define OSMAC                /* This platform operates under the MacIntosh */
#define SHORTMEMO            /* This platform has a short memory */
#define LITTLENDER   0       /* This platform does not have little-ender sex */
#define NEEDSPACKF           /* This platform needs packing functions */
#define CRUNTYPE     5       /* Runtime host C is KR4 */
#define LONG    int          /* Signed 4-byte integers */
#define UBYTE unsigned char  /* Unsigned 1-byte integer */
#define ULONG   unsigned int /* Unsigned 4-byte integers */
#define USHORT  unsigned short /* Unsigned 2-byte integers */
#endif /* MPWPLAT */

#ifdef TCCPLAT               /* Turbo C for the IBM PC DOS 2+. */
#define OSMSDOS              /* This platform operates under MS-DOS */
#define SHORTMEMO            /* This platform has a short memory */
#define FPROTOTYPE           /* This platform uses function prototypes */
#define LITTLENDER   1       /* This platform does have little-ender sex */
#define CRUNTYPE     0       /* Runtime host C is ANSI2 */
   typedef long LONG;           /* Signed 4-byte integers */
   typedef unsigned char UBYTE; /* Unsigned 1-byte integer */
   typedef unsigned long ULONG; /* Unsigned 4-byte integers */
   typedef unsigned short USHORT;  /* Unsigned 2-byte integers */
#endif /* TCCPLAT */

#ifdef THCPLAT               /* Think C for the Apple Macintosh. */
#define OSMAC                /* This platform operates under the MacIntosh */
#define SHORTMEMO            /* This platform has a short memory */
#define LITTLENDER   0       /* This platform does have little-ender sex */
#define NEEDSPACKF           /* This platform needs packing functions */
#define CRUNTYPE     5       /* Runtime host C is KR4 */
#define LONG    int          /* Signed 4-byte integers */
#define UBYTE unsigned char  /* Unsigned 1-byte integer */
#define ULONG   unsigned int /* Unsigned 4-byte integers */
#define USHORT  unsigned short /* Unsigned 2-byte integers */
#endif /* THCPLAT */

#ifdef TSOPLAT               /* IBM mainframe under TSO using SAS C */
#define OSMSDOS              /* This platform operates under MS-DOS */
#define LITTLENDER   1       /* This platform does have little-ender sex */
#define CRUNTYPE     1       /* Runtime host C is ANSI4 */
   typedef int LONG;            /* Signed 4-byte integers */
   typedef unsigned char UBYTE; /* Unsigned 1-byte integer */
   typedef unsigned int ULONG;  /* Unsigned 4-byte integers */
   typedef unsigned short USHORT;   /* Unsigned 2-byte integers */
#endif /* TSOPLAT */

#ifdef VMSPLAT               /* A VAX under VMS using standard VAX C */
#define OSVMS                /* This platform operates under VMS */
#define LITTLENDER   1       /* This platform does have little-ender sex */
#define NUCLEUS      1       /* This platform supports NUCLEUS dbs */
#define VARUNIXV             /* Use Unix variable number of parameters */
#define CRUNTYPE     3       /* Runtime host C is VMS */
   typedef int LONG;            /* Signed 4-byte integers */
   typedef unsigned char UBYTE; /* Unsigned 1-byte integer */
   typedef unsigned int ULONG;  /* Unsigned 4-byte integers */
   typedef unsigned short USHORT;   /* Unsigned 2-byte integers */
#endif /* VMSPLAT */

#ifdef SKYPLAT               /* A SKYBOLT board with Sky High C */
#define OSUNIX               /* This platform operates under UNIX */
#define FPROTOTYPE           /* This platform uses function prototypes */
#define LITTLENDER   0       /* This platform does not have little-ender sex */
#define NEEDSPACKF           /* This platform needs packing functions */
#define CRUNTYPE     6       /* Runtime host C is I860 */
   typedef int LONG;            /* Signed 4-byte integers */
   typedef unsigned char UBYTE; /* Unsigned 1-byte integer */
   typedef unsigned int ULONG;  /* Unsigned 4-byte integers */
   typedef unsigned short USHORT;   /* Unsigned 2-byte integers */
#endif /*SKYPLAT */

#ifdef SN4PLAT               /* A SUN 4 or SUN Sparcstation under SUNOS */
#define OSUNIX               /* This platform operates under UNIX */
#define NEEDMMOVE            /* This platform needs an overlaying move */
#define LITTLENDER   0       /* This platform does not have little-ender sex */
#define NEEDSPACKF           /* This platform needs packing functions */
#define VARUNIXV             /* Use Unix variable number of parameters */
#define UNXPLAT              /* This platform is generic UNIX otherwise */
#define TERMCAP              /* This platform uses the termcap facility */
#define CRUNTYPE     5       /* Runtime host C is KR4 */
#define LONG    int          /* Signed 4-byte integers */
#define UBYTE unsigned char  /* Unsigned 1-byte integer */
#define ULONG   unsigned int /* Unsigned 4-byte integers */
#define USHORT  unsigned short /* Unsigned 2-byte integers */
#endif /*SN4PLAT */

#ifdef FUJPLAT               /* A Fujitsu DS station */
#define OSUNIX               /* This platform operates under UNIX */
#define FPROTOTYPE           /* This platform uses function prototypes */
#define LITTLENDER   0       /* This platform does not have little-ender sex */
#define NEEDSPACKF           /* This platform needs packing functions */
#define ANSPLAT              /* This platform supports ANSI C */
#define CRUNTYPE      1      /* Runtime host C is ANSI4 */
   typedef int LONG;            /* Signed 4-byte integers */
   typedef unsigned char UBYTE; /* Unsigned 1-byte integer */
   typedef unsigned short USHORT;   /* Unsigned 2-byte integers */
#endif /*FUJPLAT */

#ifdef AN6PLAT               /* A ANSI C compiler on a 386 UNIX platform */
#define OSUNIX               /* This platform operates under UNIX */
#define FPROTOTYPE           /* This platform uses function prototypes */
#define LITTLENDER   1       /* This platform does have little-ender sex */
#define ANSPLAT              /* This platform supports ANSI C */
#define CRUNTYPE      1      /* Runtime host C is ANSI4 */
   typedef int LONG;            /* Signed 4-byte integers */
   typedef unsigned char UBYTE; /* Unsigned 1-byte integer */
   typedef unsigned int ULONG;  /* Unsigned 4-byte integers */
   typedef unsigned short USHORT;   /* Unsigned 2-byte integers */
#define UNXPLAT              /* This platform is generic UNIX otherwise */
#endif /*AN6PLAT */

#ifdef UCSPLAT               /* A Unisys ANSI */
#define FPROTOTYPE           /* This platform uses function prototypes */
#define LITTLENDER   1       /* This platform does have little-ender sex */
#define ANSPLAT              /* This platform supports ANSI C */
#define NEEDSPACKF           /* This platform needs packing functions */
#define CRUNTYPE      1      /* Runtime host C is ANSI4 */
   typedef int LONG;            /* Signed 4-byte integers */
   typedef unsigned char UBYTE; /* Unsigned 1-byte integer */
   typedef unsigned int ULONG;  /* Unsigned 4-byte integers */
   typedef unsigned short USHORT;   /* Unsigned 2-byte integers */
#endif /*UCSPLAT */

#ifdef ALFPLAT               /* DEC Alpha 64-bit computer */
#define AN4PLAT              /* Do it just like AN4PLAT */
#endif /* ALFPLAT */

#ifdef AN4PLAT               /* A ANSI C compiler on a sun 4 */
#define OSUNIX               /* This platform operates under UNIX */
#define LITTLENDER   0       /* This platform does not have little-ender sex */
#define NEEDSPACKF           /* This platform needs packing functions */
#define FPROTOTYPE           /* This platform uses function prototypes */
#define ANSPLAT              /* This platform supports ANSI C */
#define CRUNTYPE     1       /* Runtime host C is ANSI4 */
   typedef int LONG;            /* Signed 4-byte integers */
   typedef unsigned char UBYTE; /* Unsigned 1-byte integer */
   typedef unsigned int ULONG;  /* Unsigned 4-byte integers */
   typedef unsigned short USHORT;   /* Unsigned 2-byte integers */
#endif /*AN4PLAT */

#ifdef HUXPLAT               /* HP 9000 under Unix 5.3 */
#define OSUNIX               /* This platform operates under UNIX */
#define LITTLENDER   0       /* This platform does not have little-ender sex */
#define NEEDSPACKF           /* This platform needs packing functions */
#define VARUNIXV             /* Use Unix variable number of parameters */
#define UNXPLAT              /* This platform is generic UNIX otherwise */
#define CRUNTYPE     5       /* Runtime host C is KR4 */
#define LONG    int          /* Signed 4-byte integers */
#define UBYTE unsigned char  /* Unsigned 1-byte integer */
#define ULONG   unsigned int /* Unsigned 4-byte integers */
#define USHORT  unsigned short /* Unsigned 2-byte integers */
#endif /*HUXPLAT */

#ifdef U86PLAT               /* Unix system an a 386 workstation */
#define OSUNIX               /* This platform operates under UNIX */
#define LITTLENDER   1       /* This platform does have little-ender sex */
#define NEEDMMOVE            /* This platform needs an overlaying move */
#define VARUNIXV             /* Use Unix variable number of parameters */
#define UNXPLAT              /* This platform is generic UNIX otherwise */
#define CRUNTYPE     5       /* Runtime host C is KR4 */
#define LONG    int          /* Signed 4-byte integers */
#define UBYTE unsigned char  /* Unsigned 1-byte integer */
#define ULONG   unsigned int /* Unsigned 4-byte integers */
#define USHORT  unsigned short /* Unsigned 2-byte integers */
#endif /*U86PLAT */

#ifdef DGEPLAT               /* A Data General with an ANSI C compiler */
#define NEEDMMOVE            /* This platform needs an overlaying move */
#define NEEDSPACKF           /* This platform needs packing functions */
#define ANSI
#define FPROTOTYPE           /* This platform uses function prototypes */
#define LITTLENDER   0       /* This platform does not have little-ender sex */
#define ANSPLAT              /* This platform supports ANSI C */
#define CRUNTYPE      1      /* Runtime host C is ANSI4 */
   typedef int LONG;            /* Signed 4-byte integers */
   typedef unsigned char UBYTE; /* Unsigned 1-byte integer */
   typedef unsigned int ULONG;  /* Unsigned 4-byte integers */
   typedef unsigned short USHORT;   /* Unsigned 2-byte integers */
#endif /* DGEPLAT */


#ifdef LCCPLAT               /* Lattice C for the IBM PC DOS 2+ */
#define OSMSDOS              /* This platform operates under MS-DOS */
#define SHORTMEMO            /* This platform has a short memory */
#define FPROTOTYPE           /* This platform uses function prototypes */
#define LITTLENDER   1       /* This platform does have little-ender sex */
#define CRUNTYPE     0       /* Runtime host C is ANSI2 */
   typedef long LONG;           /* Signed 4-byte integers */
   typedef unsigned char UBYTE; /* Unsigned 1-byte integer */
   typedef unsigned long ULONG; /* Unsigned 4-byte integers */
   typedef unsigned short USHORT;  /* Unsigned 2-byte integers */
#endif /* LCCPLAT */

#ifdef COHPLAT               /* COHERENT 386 from Mark Williams Company */
#define OSUNIX               /* This platform operates under UNIX */
#define LITTLENDER   0       /* This platform does not have little-ender sex */
#define NEEDMMOVE            /* This platform needs an overlaying move */
#define VARUNIXV             /* Use Unix variable number of parameters */
#define UNXPLAT              /* This platform is generic UNIX otherwise */
#define CRUNTYPE     5       /* Runtime host C is KR4 */
#endif /*COHPLAT */

#ifdef USETXTFNC
#ifndef PRIOFUNC
#define PRIOFUNC
#endif
#endif

#include <stdio.h>
#ifndef TRUE
#define true   1
#endif
#ifndef FALSE
#define false 0
#endif
#define EPSILON_FUZZ 1.0e-35      /* Epsilon fuzz value */
#define LMADDR   unsigned int     /* Type of indexes into long memory */

#ifdef FPROTOTYPE
   void Debug(char* FormatString,...);
   void DebugSet(int status);
   void DebugSetname(char* Filename);
   void LogMessage(char* FormatString,...);
#define VOID void
// #define HANDLE void*
#define CONST const
#else
#define VOID char
// #define HANDLE char*
#define CONST
#endif

#ifdef CHARTYPE
#define CLETTER         1
#define CUPPERC         2
#define CLOWERC         4
#define CIDENTC         8
#define CNUMBER        16
#define CSYMBOL        32
#define CWSPACE        64
#define CQUOTEC       128       /* Indicates a quote character type */

#define CEMPTYC        16       /* DLE Indicates empty character */
#define NULLCHAR       25       /* EM  Indicates numm character */
#define BACKSLASH      26       /* SUB Metacharacter code for a backslash */
#define QUOTECHAR      28       /* FS  Metacharacter code for a double quote */
#define SQUOTECHAR     29       /* GS  Metacharacter code for a single quote */
#define SNGDBLCHAR     30       /* RS  Metacharacter code for single or double */
#endif /* CHARTYPE */

#ifdef DATETIME
#ifdef AZTPLAT
#include <utime.h>
#else
#include <time.h>
#ifdef UNXPLAT
#include <sys/types.h>
#include <sys/timeb.h>
#endif /* UNXPLAT */
#endif /* AZTPLAT */
#endif /* DATETIME */

#ifdef ECVTFUNC
#ifdef DGEPLAT
#define DIGPREC     18       /* max # of significant digits */
#endif
#ifdef UNXPLAT
#define DIGPREC     18       /* max # of significant digits */
#endif /* UNXPLAT */
#ifdef AIXPLAT
#define HIGHEXPO             /* has high precision exponent */
#define DIGPREC     18       /* max # of significant digits */
#endif /* AIXPLAT */
#ifdef FUJPLAT
#define HIGHEXPO             /* has high precision exponent */
#define DIGPREC     18       /* max # of significant digits */
#endif /* FUJPLAT */
#ifdef A86PLAT
#define HIGHEXPO             /* has high precision exponent */
#define DIGPREC     18       /* max # of significant digits */
#endif /* A86PLAT */
#ifdef AN4PLAT
#define HIGHEXPO             /* has high precision exponent */
#define DIGPREC     18       /* max # of significant digits */
#endif /* AN4PLAT */
#ifdef UCSPLAT
#define HIGHEXPO             /* has high precision exponent */
#define DIGPREC     18       /* max # of significant digits */
#endif /* UCSPLAT */
#ifdef AZTPLAT
#define DIGPREC     17       /* max # of significant digits */
#endif /* AZTPLAT */
#ifdef MPWPLAT
#define DIGPREC     17       /* max # of significant digits */
#endif /* MPWPLAT */
#ifdef TCCPLAT
#include <string.h>          /* declarations for "memset+memcpy" */
#include <stdlib.h>          /* declaration for "ecvt" */
#define HASECVTF             /* platform has ecvt */
#endif /* TCCPLAT */
#ifdef THCPLAT
#define DIGPREC     17       /* max # of significant digits */
#endif /* THCPLAT */
#ifdef TSOPLAT
#define DIGPREC     17       /* max # of significant digits */
#endif /* TSOPLAT */
#ifdef VMSPLAT
#include <string.h>          /* declarations for "memset+memcpy" */
#include <stdlib.h>          /* declaration for "ecvt" */
#define HASECVTF             /* platform has ecvt */
#endif /* VMSPLAT */
#ifdef SKYPLAT
#define HIGHEXPO              /* has high precision exponent */
#define DIGPREC     18        /* max # of significant digits */
#endif /* SKYPLAT */
#ifdef LCCPLAT
#include <string.h>           /* declarations for "memset+memcpy" */
#include <math.h>             /* declaration for "ecvt" */
#define HASECVTF              /* platform has ecvt */
#endif /* LCCPLAT */
#endif /* ECVTFUNC */

#ifdef LONGMEMO
#ifdef HASLNGFN
   LONG* defptr(unsigned int);
   LONG* prcptr(unsigned int);
   LONG* valptr(unsigned int);
   unsigned int defadr(LONG*);
   unsigned int prcadr(LONG*);
#else
   extern LONG* defn;            /* Definition storage area */
   extern LONG* proc;            /* Procedure storage area */
   extern LONG* valu;            /* Value storage area */
#define defptr(x) (defn + x)
#define prcptr(x) (proc + x)
#define valptr(x) (valu + x)
#ifdef THCPLAT
#define defadr(x) ((unsigned int)(x - defn))
#define prcadr(x) ((unsigned int)(x - proc))
#else
#define defadr(x) (x - defn)
#define prcadr(x) (x - proc)
#endif /* THCPLAT */
#endif /* HASLNGFN */
#endif /* LONGMEMO */

#ifdef MATHFUNC
#ifdef MPWPLAT
#define LONGDBL
#endif /* MPWPLAT */
#include <math.h>
#ifdef DGEPLAT
#define BIGDBLE   1.0e75
#else
#ifdef VMSPLAT
#define BIGDBLE   1.0e38
#else
#define BIGDBLE   1.0e300
#endif /* VMSPLAT */
#endif /* DGEPLAT */
#endif /* MATHFUNC */

#ifdef OSYSFUNC
#ifdef UNXPLAT
   extern void exit();
   extern char* getenv();
#else
#ifdef AZTPLAT
   extern void exit();
   extern char* getenv();
#else
#ifdef VMSPLAT
   extern void exit();
   extern char* getenv();
#else
#include <setjmp.h>
#include <stdlib.h>
#endif /* VMSPLAT */
#endif /* AZTPLAT */
#endif /* UNXPLAT */
#endif /* OSYSFUNC */

#ifdef STRGFUNC
#include <string.h>
#ifdef THCPLAT
#define cmpstrn(x,y,z) memcmp(x,y,(size_t)z)
#else
#define cmpstrn  memcmp
#endif
#endif /* STRGFUNC */

#ifdef RAWMFUNC
#include <string.h>
#ifdef THCPLAT
#define cpymem(x,y,z) memcpy(y,x,(size_t)z)
#define cpyovl(x,y,z) memmove(y,x,(size_t)z)
#define filmem(p,n,v) memset(p,v,(size_t)n)
#define zeromem(p,x)  memset(p,0,(size_t)x)
#else
#define filmem(x,y,z) memset(x,z,y)
#define cpymem(x,y,z) memcpy(y,x,z)
#ifdef NEEDMMOVE
   extern void cpyovl();
#else
#define cpyovl(x,y,z) memmove(y,x,z)
#define zeromem(p,n)  memset(p,0,n)
#endif /* NEEDMMOVE */
#endif /* THCPLAT */
#endif /* RAWMFUNC */

#ifdef PACKFUNC
#ifdef MPWPLAT
#define longdbl   4    /* Number of longs per double */
#else
#define longdbl   2    /* Number of longs per double */
#endif /* MPWPLAT */
#ifdef NEEDSPACKF
#define getuns16(x,y) get16u(&(x),y)
#define getsng16(x,y) get16s(&(x),y)
#define getsng32(x,y) get32(&(x),y)
#define getuns32(x,y) get32u(&(x),y)
#define getdble(x,y) get64(&(x),y)
#ifdef FPROTOTYPE
   void get16u(unsigned int* x,unsigned char* y);
   void get16s(int* x,unsigned char* y);
   void putuns16(unsigned char* x,unsigned int y);
   void putsng16(unsigned char* x,int y);
   void get32(LONG* x,unsigned char* y);
   void get32u(ULONG* x,unsigned char* y);
   void putsng32(unsigned char* x,LONG y);
   void putuns32(unsigned char* x,unsigned int y);
   void get64(double* x,unsigned char* y);
   void putdble(unsigned char* x,double y);
#else
   extern void get16u();
   extern void get16s();
   extern void putuns16();
   extern void putsng16();
   extern void get32();
   extern void get32u();
   extern void putsng32();
   extern void putuns32();
   extern void get64();
   extern void putdble();
#endif /* FPROTOTYPE */
#else
#define getuns16(x,y)  x = *(unsigned short*)(y)
#define putuns16(x,y)  *(unsigned short*)(x) = (unsigned short) y
#define getsng16(x,y)  x = *(short*)(y)
#define putsng16(x,y)  *(short*)(x) = (short) y
#define getsng32(x,y)  x = *(LONG*)(y)
#define getuns32(x,y)  x = *(ULONG*)(y)
#define putsng32(x,y)  *(LONG*)(x) = y
#define putuns32(x,y)  *(ULONG*)(x) = (ULONG) y
#define getdble(x,y)   x = *(double*)(y)
#define putdble(x,y)   *(double*)(x) = y
#define getptr(x,y)   x = *(void**)(y)
#define valuns16(y)  *(unsigned short*)(y)
#define valsng16(y)  *(short*)(y)
#define valsng32(y)  *(LONG*)(y)
#define valdble(y)   *(double*)(y)
#endif /* NEEDSPACKF */
#endif /* PACKFUNC */

#ifdef MEMOFUNC
#ifdef UNXPLAT
#ifndef COHPLAT
# ifndef OSX
#  include <malloc.h>
# endif
#endif
#else
#ifdef AZTPLAT
   extern char* malloc();
#else
#ifdef COHPLAT
   extern char* malloc();
#else
#include <stdlib.h>
#endif /* COHPLAT */
#endif /* AZTPLAT */
#endif /* UNXPLAT */
#ifdef THCPLAT
#define getmem(x) malloc((size_t)x)
#define xgetmem(x) malloc((size_t)x)
#define xfree(x) free(x)
#else
#define getmem(x) malloc(x)
#define xgetmem(x) malloc(x)
#define xfree(x) free(x)
#endif /* THCPLAT */
#endif /* MEMOFUNC */

#ifdef PRIOFUNC
#ifdef WTCPLAT
#include <io.h>
#include <fcntl.h>
#include <sys\stat.h>
#define EOLNCHAR 2                   /* End-of-line number of characters */
#define setbufsz(x,y,z)  setvbuf(x,y,_IOFBF,z)
#define fileptr void*                /* Longest basic type needed for handle */
#define txtfile FILE*                /* The type of a text file */
#define binfile int                  /* The type of raw binary file handles */
#define SINPUT  stdin                /* The standard input file */
#define SOUTPUT stdout               /* The standard output file */
#define SCONSOL stderr               /* The console file */
#define nultxtf NULL                 /* Indicates an unassigned text file */
#define initxtf(x) us_fopen(x,"w+")     /* Initialize and open a text file */
#define apptxtf(x) us_fopen(x,"a")      /* Initialize or append to a text file */
#define opntxtf(x) us_fopen(x,"r+")     /* Open an existing text file */
#define rdotxtf(x) us_fopen(x,"r")      /* Open a readonly text file */
#ifdef USETXTFNC
#define wrtxtf  SioWrtxtf
#define wrteol  SioWrteol
#define wrtffd  SioWrtffd
#define rdtxtf  SioRdtxtf
#ifdef printf
#undef printf
#endif
#define printf  SioPrintf
#ifdef FPROTOTYPE
   int SioPrintf(char*,...);
#else
   extern int SioPrintf();
#endif
   extern void SioWrtxtf();
   extern void SioWrteol();
   extern void SioWrtffd();
   extern void* SioRdtxtf();
#else
#define wrtxtf(f,r,n) fwrite(r,1,n,f)/* Write a coded record */
#define wrteol(x)  fputc('\n',x)     /* Write an end-of-line */
#define wrtffd(x)  fputc('\014',x)   /* Write a formfeed */
#define rdtxtf(x,y,z) fgets(y,z,x)   /* Read a text file */
#endif
#define clstxtf(x) fclose(x)         /* Close a text file */
#define puttxtf(x,y) fseek(x,y,SEEK_SET)/* Position a text file */
#define rewtxtf(x) fseek(x,0L,SEEK_SET) /* Rewind a text file */
#define siztxtf(x) fseek(x,0L,SEEK_END) /* Position to end (size) of a text file */
#define postxtf(x) ftell(x)          /* Return the position of a text file */
#define txtopner(x) (x==NULL)        /* Text open error ? */
#define nulbinf    0                 /* Indicates an unassigned binary file */
#define inibinf(x) open(x,O_CREAT|O_TRUNC|O_RDWR|O_BINARY,S_IREAD|S_IWRITE)
#define opnbinf(x) open(x,O_RDWR|O_BINARY)
#define rdobinf(x) open(x,O_RDONLY|O_BINARY)
#define rdbinf(f,r,n) read(f,r,n)
#define wrbinf(f,r,n) write(f,r,n)
#define rdlong(f,r,n) read(f,r,n*4)
#define wrlong(f,r,n) write(f,r,n*4)
#define clsbinf(x) close(x)
#define rewbinf(x) lseek(x,0L,SEEK_SET)
#define putbinf(x,y) lseek(x,y,SEEK_SET)
#define posbinf(x) lseek(x,0L,SEEK_CUR)
#define sizbinf(x) lseek(x,0L,SEEK_END)
#define binopner(x) (x<0)
#define delfile(x) remove(x)
#else
#ifdef BO2PLAT
#include <io.h>
#include <fcntl.h>
#include <sys\stat.h>
#define EOLNCHAR 2                   /* End-of-line number of characters */
#define setbufsz(x,y,z)  setvbuf(x,y,_IOFBF,z)
#define fileptr void*                /* Longest basic type needed for handle */
#define txtfile FILE*                /* The type of a text file */
#define binfile int                  /* The type of raw binary file handles */
#define SINPUT  stdin                /* The standard input file */
#define SOUTPUT stdout               /* The standard output file */
#define SCONSOL stderr               /* The console file */
#define nultxtf NULL                 /* Indicates an unassigned text file */
#define initxtf(x) us_fopen(x,"w+")     /* Initialize and open a text file */
#define apptxtf(x) us_fopen(x,"a")      /* Initialize or append to a text file */
#define opntxtf(x) us_fopen(x,"r+")     /* Open an existing text file */
#define rdotxtf(x) us_fopen(x,"r")      /* Open a readonly text file */
#ifdef USETXTFNC
#define wrtxtf  SioWrtxtf
#define wrteol  SioWrteol
#define wrtffd  SioWrtffd
#define rdtxtf  SioRdtxtf
#ifdef printf
#undef printf
#endif
#define printf  SioPrintf
#ifdef FPROTOTYPE
   int SioPrintf(char*,...);
#else
   extern int SioPrintf();
#endif
   extern void SioWrtxtf();
   extern void SioWrteol();
   extern void SioWrtffd();
   extern void* SioRdtxtf();
#else
#define wrtxtf(f,r,n) fwrite(r,1,n,f)/* Write a coded record */
#define wrteol(x)  fputc('\n',x)     /* Write an end-of-line */
#define wrtffd(x)  fputc('\014',x)   /* Write a formfeed */
#define rdtxtf(x,y,z) fgets(y,z,x)   /* Read a text file */
#endif
#define clstxtf(x) fclose(x)         /* Close a text file */
#define puttxtf(x,y) fseek(x,y,SEEK_SET)/* Position a text file */
#define rewtxtf(x) fseek(x,0L,SEEK_SET) /* Rewind a text file */
#define siztxtf(x) fseek(x,0L,SEEK_END) /* Position to end (size) of a text file */
#define postxtf(x) ftell(x)          /* Return the position of a text file */
#define txtopner(x) (x==NULL)        /* Text open error ? */
#define nulbinf    0                 /* Indicates an unassigned binary file */
#define inibinf(x) open(x,O_CREAT|O_TRUNC|O_RDWR|O_BINARY,S_IREAD|S_IWRITE)
#define opnbinf(x) open(x,O_RDWR|O_BINARY)
#define rdobinf(x) open(x,O_RDONLY|O_BINARY)
#define rdbinf(f,r,n) read(f,r,n)
#define wrbinf(f,r,n) write(f,r,n)
#define rdlong(f,r,n) read(f,r,n*4)
#define wrlong(f,r,n) write(f,r,n*4)
#define clsbinf(x) close(x)
#define rewbinf(x) lseek(x,0L,SEEK_SET)
#define putbinf(x,y) lseek(x,y,SEEK_SET)
#define posbinf(x) lseek(x,0L,SEEK_CUR)
#define sizbinf(x) lseek(x,0L,SEEK_END)
#define binopner(x) (x<0)
#define delfile(x) remove(x)
#else
#ifdef UNXPLAT
#define EOLNCHAR 1                   /* End-of-line number of characters */
#define SEEK_SET  0
#define SEEK_END  2
#endif /* UNXPLAT */
#ifdef AIXPLAT
#define EOLNCHAR 1                   /* End-of-line number of characters */
#define setbufsz(x,y,z)  setvbuf(x,y,_IOFBF,z)
#endif /* AIXPLAT */
#ifdef FUJPLAT
#define EOLNCHAR 1                   /* End-of-line number of characters */
#define setbufsz(x,y,z)  setvbuf(x,y,_IOFBF,z)
#endif /* FUJPLAT */
#ifdef AN4PLAT
#define EOLNCHAR 1                   /* End-of-line number of characters */
#define setbufsz(x,y,z)  setvbuf(x,y,_IOFBF,z)
#endif /* AN4PLAT */
#ifdef A86PLAT
#define EOLNCHAR 2                   /* End-of-line number of characters */
#define setbufsz(x,y,z)  setvbuf(x,y,_IOFBF,z)
#endif /* A86PLAT */
#ifdef AZTPLAT
#define EOLNCHAR 1                   /* End-of-line number of characters */
#define SEEK_SET  0
#define SEEK_END  2
#endif /* AZTPLAT */
#ifdef MPWPLAT
#define EOLNCHAR 2                   /* End-of-line number of characters */
#define SEEK_SET  0
#define SEEK_END  2
#endif /* MPWPLAT */
#ifdef TCCPLAT
#define EOLNCHAR 2                   /* End-of-line number of characters */
#endif /* TCCPLAT */
#ifdef THCPLAT
#define EOLNCHAR 2                   /* End-of-line number of characters */
#endif /* THCPLAT */
#ifdef TSOPLAT
#define EOLNCHAR 2                   /* End-of-line number of characters */
#define SEEK_SET  0
#define SEEK_END  2
#endif /* TSOPLAT */
#ifdef VMSPLAT
#define EOLNCHAR 1                   /* End-of-line number of characters */
#endif /* VMSPLAT */
#ifdef SKYPLAT
#define EOLNCHAR 1                   /* End-of-line number of characters */
#endif /* SKYPLAT */
#ifdef LCCPLAT
#define EOLNCHAR 2                   /* End-of-line number of characters */
#endif /* LCCPLAT */
#define fileptr void*                /* Longest basic type needed for handle */
#define txtfile FILE*                /* The type of a text file */
#define binfile FILE*                /* The type of raw binary file handles */
#define SINPUT  stdin                /* The standard input file */
#define SOUTPUT stdout               /* The standard output file */
#define SCONSOL stderr               /* The console file */
#define nultxtf NULL                 /* Indicates an unassigned text file */
#define initxtf(x) us_fopen(x,"w+")     /* Initialize and open a text file */
#define apptxtf(x) us_fopen(x,"a")      /* Initialize or append to a text file */
#define opntxtf(x) us_fopen(x,"r+")     /* Open an existing text file */
#define rdotxtf(x) us_fopen(x,"r")      /* Open a readonly text file */
#ifdef USETXTFNC
#define wrtxtf  SioWrtxtf
#define wrteol  SioWrteol
#define wrtffd  SioWrtffd
#define rdtxtf  SioRdtxtf
#ifdef printf
#undef printf
#endif
#define printf  SioPrintf
#ifdef FPROTOTYPE
   int SioPrintf(char*,...);
#else
   extern int SioPrintf();
#endif
   extern void SioWrtxtf();
   extern void SioWrteol();
   extern void SioWrtffd();
   extern void* SioRdtxtf();
#else
#define rdtxtf(x,y,z) fgets(y,z,x)   /* Read a text file */
#define wrteol(x)  fputc('\n',x)     /* Write an end-of-line */
#define wrtffd(x)  fputc('\014',x)   /* Write a formfeed */
#ifdef THCPLAT
#define wrtxtf(f,r,n) fwrite(r,1L,(size_t)n,f)
#else
#ifdef VMSPLAT
#define wrtxtf(f,r,n) fwrite(r,n,1,f)/* Write a coded record */
#else
#define wrtxtf(f,r,n) fwrite(r,1,n,f)/* Write a coded record */
#endif /* VMSPLAT */
#endif /* THCPLAT */
#endif
#define clstxtf(x) fclose(x)         /* Close a text file */
#define puttxtf(x,y) fseek(x,y,SEEK_SET)/* Position a text file */
#define rewtxtf(x) fseek(x,0L,SEEK_SET) /* Rewind a text file */
#define siztxtf(x) fseek(x,0L,SEEK_END) /* Position to end (size) of a text file */
#define postxtf(x) ftell(x)          /* Return the position of a text file */
#define txtopner(x) (x==NULL)        /* Text open error ? */
#define nulbinf    NULL              /* Indicates an unassigned binary file */
#define inibinf(x) us_fopen(x,"w+b")    /* Initialize and open a binary file */
#define opnbinf(x) us_fopen(x,"r+b")    /* Open an existing binary file */
#define appbinf(x) us_fopen(x,"a+b")    /* Append to a binary file */
#define rdobinf(x) us_fopen(x,"rb")     /* Open a readonly binary file */
#ifdef THCPLAT
#define rdbinf(f,r,n) fread(r,1L,(size_t)n,f)
#define wrbinf(f,r,n) fwrite(r,1L,(size_t)n,f)
#define rdlong(f,r,n) fread(r,(size_t)n,4L,f)
#define wrlong(f,r,n) fwrite(r,(size_t)n,4L,f)
#else
#define rdbinf(f,r,n) fread(r,1,n,f) /* Read a binary file */
#define wrbinf(f,r,n) fwrite(r,1,n,f)/* Write a binary file */
#define rdlong(f,r,n) fread(r,n,4,f) /* Read longs from a binary file */
#define wrlong(f,r,n) fwrite(r,n,4,f)/* Write longs to a binary file */
#endif /* THCPLAT */
#define clsbinf(x) fclose(x)         /* Close a binary file */
#define rewbinf(x) fseek(x,0L,SEEK_SET) /* Rewind a binary file */
#define putbinf(x,y) fseek(x,y,SEEK_SET) /* Position a binary file */
#define posbinf(x) ftell(x)          /* Position of a binary file */
#define sizbinf(x) fseek(x,0L,SEEK_END)  /* Size of a binary file */
#define binopner(x) (x==NULL)         /* Binary open error ? */
#ifdef VMSPLAT
#define delfile(x) delete(x)         /* Delete a file */
#else
#ifdef SKYPLAT
#define delfile(x) remove(x)         /* Delete a file */
#else
#ifdef AN4PLAT
#define delfile(x) remove(x)         /* Delete a file */
#else
#ifdef A86PLAT
#define delfile(x) remove(x)         /* Delete a file */
#else
#ifdef ZORPLAT
#define delfile(x) remove(x)         /* Delete a file */
#else
#ifdef ANSPLAT
#define delfile(x) remove(x)         /* Delete a file */
#else
#define delfile(x) QFile::remove(x)         /* Delete a file */
#endif /* ANSPLAT */
#endif /* ZORPLAT */
#endif /* A86PLAT */
#endif /* AN4PLAT */
#endif /* SKYPLAT */
#endif /* VMSPLAT */
#endif /* BO2PLAT */
#endif /* WTCPLAT */
#endif /* PRIOFUNC */

#ifdef UBIOFUNC
#ifdef UNXPLAT
#define rawfile FILE*                /* The type of raw binary file handles */
#define nulrawf    NULL              /* Indicates an unassigned binary file */
#define inirawf(x) us_fopen(x,"w+b")    /* Initialize and open a binary file */
#define opnrawf(x) us_fopen(x,"r+b")    /* Open an existing binary file */
#define rdorawf(x) us_fopen(x,"rb")    /* Open an existing binary file */
#define rdrawf(f,r,n) fread(r,1,n,f) /* Read a binary file */
#define wrrawf(f,r,n) fwrite(r,1,n,f)/* Write a binary file */
#define clsrawf(x) fclose(x)         /* Close a binary file */
#define rewrawf(x) fseek(x,0L,0)     /* Rewind a binary file */
#define putrawf(x,y) fseek(x,y,0)    /* Position a binary file */
#define posrawf(x) ftell(x)          /* Position of a binary file */
#define sizrawf(x) fseek(x,0L,2)     /* Size of a binary file */
#define rawopner(x) (x==NULL)        /* Binary open error ? */
#endif /* UNXPLAT */
#ifdef ANSPLAT
#define rawfile FILE*                /* The type of raw binary file handles */
#define nulrawf    NULL              /* Indicates an unassigned binary file */
#define inirawf(x) us_fopen(x,"w+b")    /* Initialize and open a binary file */
#define opnrawf(x) us_fopen(x,"r+b")    /* Open an existing binary file */
#define rdorawf(x) us_fopen(x,"rb")    /* Open an existing binary file */
#define rdrawf(f,r,n) fread(r,1,n,f) /* Read a binary file */
#define wrrawf(f,r,n) fwrite(r,1,n,f)/* Write a binary file */
#define clsrawf(x) fclose(x)         /* Close a binary file */
#define rewrawf(x) fseek(x,0L,0)     /* Rewind a binary file */
#define putrawf(x,y) fseek(x,y,0)    /* Position a binary file */
#define posrawf(x) ftell(x)          /* Position of a binary file */
#define sizrawf(x) fseek(x,0L,2)     /* Size of a binary file */
#define rawopner(x) (x==NULL)        /* Binary open error ? */
#endif /* ANSPLAT */
#ifdef AZTPLAT
#include <fcntl.h>
   extern long lseek();
#define rawfile int                /* The type of raw binary file handles */
#define nulrawf 0                  /* Indicates an unassigned binary file */
#define inirawf(x) open(x,O_CREAT|O_TRUNC|O_RDWR) /* Initialize and open a binary file */
#define opnrawf(x) open(x,O_RDWR)   /* Open an existing binary file */
#define rdorawf(x) open(x,O_RDONLY) /* Open an existing binary file */
#define rdrawf(f,r,n) read(f,r,n)   /* Read a binary file */
#define wrrawf(f,r,n) write(f,r,n)  /* Write a binary file */
#define clsrawf(x) close(x)         /* Close a binary file */
#define rewrawf(x) lseek(x,0L,0)    /* Rewind a binary file */
#define putrawf(x,y) lseek(x,y,0)   /* Position a binary file */
#define posrawf(x) lseek(x,0L,1)    /* Position of a binary file */
#define sizrawf(x) lseek(x,0L,2)    /* Size of a binary file */
#define rawopner(x) (x<0)           /* Binary open error ? */
#endif /* AZTPLAT */
#ifdef MPWPLAT
#define rawfile FILE*                /* The type of raw binary file handles */
#define nulrawf    NULL              /* Indicates an unassigned binary file */
#define inirawf(x) us_fopen(x,"w+b")    /* Initialize and open a binary file */
#define opnrawf(x) us_fopen(x,"r+b")    /* Open an existing binary file */
#define rdorawf(x) us_fopen(x,"rb")     /* Open an existing binary file */
#define rdrawf(f,r,n) fread(r,1,n,f) /* Read a binary file */
#define wrrawf(f,r,n) fwrite(r,1,n,f)/* Write a binary file */
#define clsrawf(x) fclose(x)         /* Close a binary file */
#define rewrawf(x) fseek(x,0L,0)     /* Rewind a binary file */
#define putrawf(x,y) fseek(x,y,0)    /* Position a binary file */
#define posrawf(x) ftell(x)          /* Position of a binary file */
#define sizrawf(x) fseek(x,0L,2)     /* Size of a binary file */
#define rawopner(x) (x==NULL)        /* Binary open error ? */
#endif /* MPWPLAT */
#ifdef TCCPLAT
#include <io.h>
#include <fcntl.h>                   /* File control options for open() */
#include <sys\stat.h>                /* Defines structures for stat */
#define rawfile int                  /* The type of raw binary file handles */
#define nulrawf    -1                /* Indicates an unassigned binary file */
#define inirawf(x) open(x,O_CREAT | O_TRUNC | O_RDWR | O_BINARY,S_IWRITE | S_IREAD)
#define opnrawf(x) open(x,O_RDWR | O_BINARY)
#define rdorawf(x) open(x,O_RDONLY | O_BINARY)
#define rdrawf(x,y,z)  read(x,y,z)   /* Read from a binary file */
#define wrrawf(x,y,z) write(x,y,z)   /* Write to a binary file */
#define clsrawf(x)  close(x)         /* Close a binary file */
#define rewrawf(x) lseek(x,0L,0)     /* Rewind a binary file */
#define putrawf(x,y) lseek(x,y,0)    /* Position a binary file */
#define posrawf(x) lseek(x,0L,1)     /* Return position of a binary file */
#define sizrawf(x) lseek(x,0L,2)     /* Size of a binary file */
#define rawopner(x) (x<0)            /* Binary open error ? */
#endif /* TCCPLAT */
#ifdef THCPLAT
#include <unix.h>
#include <fcntl.h>
#define rawfile int                  /* The type of raw binary file handles */
#define nulrawf    -1                /* Indicates an unassigned binary file */
#define inirawf(x) open(x,O_CREAT|O_TRUNC|O_RDWR|O_BINARY)
#define opnrawf(x) open(x,O_RDWR|O_BINARY)
#define rdorawf(x) open(x,O_RDONLY|O_BINARY)
#define rdrawf(x,y,z)  read(x,y,z)   /* Read from a binary file */
#define wrrawf(x,y,z) write(x,y,z)   /* Write to a binary file */
#define clsrawf(x)  close(x)         /* Close a binary file */
#define rewrawf(x) lseek(x,0L,SEEK_SET) /* Rewind a binary file */
#define putrawf(x,y) lseek(x,y,SEEK_SET)/* Position a binary file */
#define posrawf(x) lseek(x,0L,SEEK_CUR) /* Return position of a binary file */
#define sizrawf(x) lseek(x,0L,SEEK_END) /* Size of a binary file */
#define rawopner(x) (x<0)            /* Binary open error ? */
#endif /* THCPLAT */
#ifdef TSOPLAT
#define rawfile FILE*                /* The type of raw binary file handles */
#define nulrawf    NULL              /* Indicates an unassigned binary file */
#define inirawf(x) us_fopen(x,"w+b")    /* Initialize and open a binary file */
#define opnrawf(x) us_fopen(x,"r+b")    /* Open an existing binary file */
#define rdorawf(x) us_fopen(x,"rb")     /* Open an existing binary file */
#define rdrawf(f,r,n) fread(r,1,n,f) /* Read a binary file */
#define wrrawf(f,r,n) fwrite(r,1,n,f)/* Write a binary file */
#define clsrawf(x) fclose(x)         /* Close a binary file */
#define rewrawf(x) fseek(x,0L,0)     /* Rewind a binary file */
#define putrawf(x,y) fseek(x,y,0)    /* Position a binary file */
#define posrawf(x) ftell(x)          /* Position of a binary file */
#define sizrawf(x) fseek(x,0L,2)     /* Size of a binary file */
#define rawopner(x) (x==NULL)        /* Binary open error ? */
#endif /* TSOPLAT */
#ifdef VMSPLAT
#define rawfile FILE*                /* The type of raw binary file handles */
#define nulrawf    NULL
#define inirawf(x) us_fopen(x,"w+b")    /* Initialize a binary file */
#define opnrawf(x) us_fopen(x,"r+b")    /* Open an existing binary file */
#define rdorawf(x) us_fopen(x,"rb")     /* Open an existing binary file */
#define rdrawf(f,r,n) fread(r,1,n,f) /* Read a binary file */
#define wrrawf(f,r,n) fwrite(r,1,n,f)/* Write a binary file */
#define clsrawf(x) fclose(x)         /* Close a binary file */
#define rewrawf(x) fseek(x,0L,SEEK_SET)     /* Rewind a binary file */
#define putrawf(x,y) fseek(x,y,SEEK_SET)    /* Position a binary file */
#define posrawf(x) ftell(x)         /* Position of a binary file */
#define sizrawf(x) fseek(x,0L,SEEK_END)     /* Size of a binary file */
#define rawopner(x) (x==NULL)        /* Binary open error ? */
#endif /* VMSPLAT */
#ifdef SKYPLAT
#include <system.h>
#include <fcntl.h>                   /* File control options for open() */
#include <sys/stdtypes.h>            /* System file information types */
#include <sys/stat.h>                /* Defines structures for stat */
#define rawfile int                  /* The type of raw binary file handles */
#define nulrawf    -1                /* Indicates an unassigned binary file */
#define inirawf(x) open(x,O_CREAT|O_TRUNC|O_RDWR,S_IWRITE|S_IREAD)
#define opnrawf(x) open(x,O_RDWR)
#define rdorawf(x) open(x,O_RDONLY)
#define rdrawf(x,y,z)  read(x,y,z)   /* Read from a binary file */
#define wrrawf(x,y,z)  write(x,y,z)  /* Write to a binary file */
#define clsrawf(x)     close(x)         /* Close a binary file */
#define rewrawf(x)     lseek(x,0L,SEEK_SET) /* Rewind a binary file */
#define putrawf(x,y)   lseek(x,y,SEEK_SET)/* Position a binary file */
#define posrawf(x) lseek(x,0L,SEEK_CUR) /* Return position of a binary file */
#define sizrawf(x)     lseek(x,0L,SEEK_END) /* Size of a binary file */
#define rawopner(x) (x<0)         /* Binary open error ? */
#endif /* SKYPLAT */
#ifdef LCCPLAT
#ifdef UBIOFUNC
#include "fcntl.h"                   /* File control options for open() */
#define rawfile int                  /* The type of raw binary file handles */
#define nulrawf    -1                /* Indicates an unassigned binary file */
#define inirawf(x) open(x,O_CREAT | O_TRUNC | O_RDWR | O_RAW,S_IWRITE | S_IREAD)
#define opnrawf(x) open(x,O_RDWR | O_RAW)
#define rdorawf(x) open(x,O_RDONLY | O_RAW)
#define rdrawf(x,y,z)  read(x,y,z)   /* Read from a binary file */
#define wrrawf(x,y,z) write(x,y,z)   /* Write to a binary file */
#define clsrawf(x)  close(x)         /* Close a binary file */
#define rewrawf(x) lseek(x,0L,0) /* Rewind a binary file */
#define putrawf(x,y) lseek(x,y,0)/* Position a binary file */
#define posrawf(x) lseek(x,0L,1) /* Return position of a binary file */
#define sizrawf(x) lseek(x,0L,2) /* Size of a binary file */
#define rawopner(x) (x<0)         /* Binary open error ? */
#endif
#endif /* LCCPLAT */
#endif /* UBIOFUNC */

#ifdef THRDFUNC
#define TLS _declspec( thread )
#endif /* THRDFUNC */

#ifdef VARARGUS
#ifdef VARUNIXV
#include <varargs.h>
#else
#include <stdarg.h>
#endif /* VARUNIXV */
#endif /* VARARGUS */

#ifdef SERVICES
   typedef int   (*IntegerService)();
   typedef void*  (*HandleService)();
   typedef void   (*VoidService)();
   typedef int*   (*IVectorService)();
   typedef char*  (*StringService)();
   typedef double (*DoubleService)();
#endif

#endif  /* platform_h */

   /*
     ;/hdr/ ************************************************************************
     ;
     ; Copyright 1986-1998 Promula Development Corporation ALL RIGHTS RESERVED
     ;
     ; name of module: pfclib.h -- Promula.Fortran Runtime Library Header
     ;
     ; This header file contains all the symbols and typedefs needed to work with
     ; the FORTRAN runtime library files. It is usually kept in the same location
     ; as the source for the library and is therefore included with "quotes". Note
     ; that this header is platform independent and therefore must be included
     ; after the platform header.
     ;
     ; global symbols defined:
     ;
     ; useful notes and assumptions:
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; edit 06/17/98 FKG: Changed names of global symbols to conform to the
     ;                    Promula version 7.08 API naming conventions.
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00
     ;
     ;/hdr/ ************************************************************************
   */

#ifdef PRIOFUNC
#define FIOMAX    256        /* Maximum number of open units */
#define FIONMAX   128        /* Maximum size of a filename */
#define BLANKNULL   1        /* Use BLANK = NULL as OPEN default */

   typedef struct {
      int funit;               /* file unit number */
      unsigned int fstat;      /* file status flags */
      unsigned int fstat1;     /* additional file status flags */
      char fname[FIONMAX];     /* file name at open */
      fileptr ffile;           /* file handle for C system */
      LONG* fassov;            /* file associated variable */
      LONG frsize;             /* file record size */
      LONG frecrd;             /* file current record number or position */
      LONG fmaxrc;             /* file maximum number of records */
      char* fassob;            /* file associated buffer */
      LONG fbufsiz;            /* file associated buffer size */
      char* fuserinf;          /* user particular information */
      int fuser;               /* user flags */
   } FIOFILE;

#define FSCRATCH    1        /* scratch file flag */
#define FDIRECT     2        /* direct access file flag */
#define FBINARY     4        /* unformatted text flag */
#define FBLANK      8        /* blanks are nulls flag */
#define FEXIST     16        /* file already exists */
#define FUNKNOWN   32        /* file existence unknown */
#define FINTERNAL  64        /* file is an internal file */
#define FNAMED    128        /* file has been created by name only */
#define FUNUSED   256        /* file structure is not now being used */
#define FREADONLY 512        /* file is open for readonly access */
#define FKEEP    1024        /* keep file after closing */
#define FTERMIO  2048        /* file is connected to terminal for in/out */
#define FENDFIL  4096        /* file is currently positioned at end-of-file */
#define FCARRIG  8192        /* file is using carriage control */
#define FDELETE 16384        /* file is to be deleted when closed */
#define FNOCRLF     1        /* do not issue newline with write on file */
#define FBLKSIZE    2        /* file has buffer block size specified */
#define FTERMINAL   4        /* file is a terminal */
#define FFIXEDFRM   8        /* file has fixed format records */
#define FREADLAST  16        /* last operation on file was a read */
#define FALREADYO  32        /* file is already open on an OPEN */
#define FSPECUSE   64        /* file has a special use */
#define FFIXEDTY  128        /* file has fixed record type */
#define FNEEDCRLF 256        /* file needs a newline prior to write */
#define FECHOOFF  512        /* file does not echo its input */
#define FCOMLINE 1024        /* file created from command line */
#define FREREAD  2048        /* the current record is to be reread */
#endif
   /*
     Define the constants used in the READ and WRITE statements.
   */
#define  FIOBMAX      5120    /* Maximum of size of I/O buffer */
#define  REREAD       -9993
#define  NAMED        -9994
#define  CONSOLE      -9995
#define  PRINTER      -9996
#define  INTERNAL     -9997
#define  INPUT        -9998
#define  OUTPUT       -9999

#ifndef pruntime_h
   /*
     Define the single precision complex type
   */
   typedef struct {
      float cr;                /* The real part of the value */
      float ci;                /* The imaginary part of the number */
   } complex;
   /*
     Define the double precision complex type
   */
   typedef struct {
      double dr;               /* The real part of the value */
      double di;               /* The imaginary part of the number */
   } dcomplex;
   /*
     Define the namelist structure
   */
   typedef struct {
      char* nmname;
      VOID* nmvalu;
      int nmtype;
      int* nmadr;
   } namelist;
#endif  /* pruntime_h */
   /*
     Define the Virtual File System parameters
   */
   typedef struct {
      int vbdel;
      int vblru;
      int vbmru;
      int vbblk;
      int vbhan;
      char vbdat[1024];
   } vmsbtyp;
   /*
     Define the descriptor format.
   */
   typedef struct {
      unsigned short Dleng;
      unsigned char Dtype;
      unsigned char Dclass;
      char* Dptr;
   } DESCRIPTOR;
   /*
     Define the string type
   */
   typedef struct {
      char* a;                 /* Pointer to the character storage */
      int n;                   /* Length of the string */
   } string;
   /*
     Define the overall dialect convention flags.
   */
#define ZDXCHAR      1    /* Dialect uses extended character set */
#define ZDCARRI      2    /* Dialect interpretes carriage control to output */
#define ZDSUBSS      4    /* Dialect checks substring lengths */
#define ZDPAUSE      8    /* Dialect expects explicit pause */
#define ZDVAXFT     16    /* Dialect expects VAX FORTRAN conventions */
#define ZDCOBOL     32    /* Dialect expects COBOL conventions */
#define ZDFCONV     64    /* Dialect needs filename converions */
#define ZDSKPRN    128    /* Dialect expects skip/print protocol */
#define ZDXCOPY    256    /* Dialect character copies external to source */
#define ZDFLEXF    512    /* Dialect expects FLEX FORTRAN conventions */
#define ZDSTRNG   1024    /* Encode expects a string variable XF4 */
   /*
     If the ISUNALIGNED variable is defined, then define the needed packing and
     unpacking functions.
   */
#ifdef ISUNALIGNED
#ifdef FPROTOTYPE
   unsigned short get16u(char* unl);
   short get16s(char* unl);
   ULONG get32u(char* unl);
   LONG get32s(char* unl);
   float getflt(char* unl);
   double getdbl(char* unl);
   void put16u(char* unl,unsigned short val);
   void put16s(char* unl,short val);
   void put32u(char* unl,ULONG val);
   void put32s(char* unl,LONG val);
   void putflt(char* unl,float val);
   void putdbl(char* unl,double val);
#else
   unsigned short get16u();
   short get16s();
   ULONG get32u();
   LONG get32s();
   float getflt();
   double getdbl();
   void put16u();
   void put16s();
   void put32u();
   void put32s();
   void putflt();
   void putdbl();
#endif
#endif
   /*
     Define the error codes used by he runtime system. Note that a value with
     1000 added triggers the END as opposed to ERR branch in I/O statements.
     This value is stripped in so far as any external use is concerned.
   */
#define ELUN_EOF  1101    /* fiolun: end of file encountered */
#define EOPN_EOF  1102    /* fioopen: end of file encountered */
#define ERBV_EOF  1103    /* fiorbiv: end of file encountered */
#define ERTX_EOF  1104    /* fiortxt: end of file */
#define EWTX_EOF  1105    /* fiowtxt: write beyond end of file */
#define ELUN_RDO   106    /* fiolun: write to readonly file */
#define EINT_NAF   107    /* fiointu: no active file structure */
#define ELUN_NAF   108    /* fiolun: no active file structure */
#define EINT_TMF   109    /* fiointu: too many files open */
#define ELUN_TMF   110    /* fiolun: too many files open */
#define ENAM_TMF   111    /* fioname: too many files open */
#define ESIO_TMF   112    /* fiostio: too many files open */
#define ECLO_PCF   113    /* fioclose: physical close failure */
#define EOPN_POF   114    /* fioopen: physical open failed */
#define ECLO_POF   115    /* fioclose: physical open failure */
#define EBCK_FRT   116    /* fioback: At front of file */
#define EBCK_DIR   117    /* fioback: direct access file */
#define ELUN_PWF   118    /* fiolun: physical write failed */
#define ERWV_PWF   119    /* fiorwbv: physical write failure */
#define EWBV_PWF   120    /* fiowbiv: physical write error */
#define EWEF_PWF   121    /* fiowef: physical write failure */
#define ERDB_IFS   122    /* fiordb: invalid format specification */
#define ERDD_IFS   123    /* fiordd: invalid format specification */
#define ERDF_IFS   124    /* fiordf: invalid format specification */
#define ERDI_IFS   125    /* fiordi: invalid format specification */
#define ERDL_IFS   126    /* fiordl: invalid format specification */
#define ERDS_IFS   127    /* fiords: invalid format specification */
#define ERDT_IFS   128    /* fiordt: invalid format specification */
#define EWRB_IFS   129    /* fiowrb: bad format specification */
#define EWRS_IFS   130    /* fiowrs: bad format specification */
#define EWRT_IFS   131    /* fiowrt: bad format specification */
#define EWVL_IFS   132    /* fiowval: bad format specification */
#define ENXF_EFS   133    /* fionxtf: end of format string */
#define ENXF_BTF   134    /* fionxtf: bad T format */
#define ENXF_BUS   135    /* fionxtf: bad B business format string */
#define ENXF_BBF   136    /* fionxtf: bad BN,Z format */
#define EOPN_LNR   137    /* fionxtf: missing width specification */
#define ENXF_DEL   138    /* fionxtf: mising terminating delimeter */
#define ENXF_HOL   139    /* fionxtf: bad holerith string */
#define ERCK_BUF   140    /* fiorchk: internal buffer exceeded */
#define ERDX_MLP   141    /* fiordx: missing left parenthesis */
#define ERDX_COM   142    /* fiordx: missing comma */
#define ERDX_MRP   143    /* fiordx: missing right parenthesis */
#define ERDZ_MLP   144    /* fiordx: missing left parenthesis */
#define ERDZ_COM   145    /* fiordx: missing comma */
#define ERDZ_MRP   146    /* fiordx: missing right parenthesis */
#define ERNL_MNI   147    /* fiornl: missing namelist identifier */
#define ERNL_MVI   148    /* fiornl: missing variable identifier */
#define ERNL_UVI   149    /* fiornl: undefined variable identifier */
#define ERNL_SSV   150    /* fiornl: subscripted scalar variable */
#define ERNL_NNS   151    /* fiornl: nonnumeric subscripts */
#define ERNL_TMS   152    /* fiornl: too many subscripts */
#define ERNL_EQL   153    /* fiornl: missing equals sign */
#define ERNL_BSI   154    /* fiornl: bad string input */
#define ERNL_MLP   155    /* fiornl: complex missing left pren */
#define ERNL_COM   156    /* fiornl: complex missing comma */
#define ERNL_MRP   157    /* fiornl: complex missing right pren */
#define ESTD_NNC   158    /* fiostod: nonnumeric character in field */
#define EOPN_BFZ   159    /* fioopen: setting buffer size failed */
#define EXCN_ICB   160    /* ftnxcons: illegal character in binary constant */
#define EXCN_ICO   161    /* ftnxcons: illegal character in octal constant */
#define EXCN_ICH   162    /* ftnxcons: illegal character in hex constant */

   extern int fioconv;               /* General dialect convention flags */

#ifdef FPROTOTYPE
   LONG    fifidint(double a);
   int     fifindex(CONST char* s,int ns,CONST char* c,int nc);
   LONG    fifipow(LONG a,LONG b);
   int     fioclose(void);
   char*(* FioConvertName)(char* fname,int leng);
   void    fiofdata(int option,void* str,int ns);
#else
   extern LONG    fifidint();        /* FORTRAN intrinsic function IDINT */
   extern int     fifindex();        /* FORTRAN intrinsic function INDEX */
   extern LONG    fifipow();         /* Compute long integer power */
   extern int     fioclose();
   extern char*(* FioConvertName)();
   extern void    fiofdata();
#endif


   // chrapi.h
   /*
     ;/hdr/ ************************************************************************
     ;
     ; ChrApi.h -- Character Manipulation Interface
     ;
     ; The character manipulation operational entity contains the basic services to
     ; move to and from character form and to manipulate character form. These
     ; services are used by practically every other operational entity in the code
     ; base. Structurally the character manipulation operational entity is
     ; considered to contain library services and has control structure associated
     ; with it.
     ;
     ; The C code which makes up the character manipulation services is fully
     ; portable and has been designed to be thread-safe as long as the various
     ; initialization services are referenced via the main thread.
     ;
     ;/hdr/ ************************************************************************
   */
#define CHR_EFORMAT              1
#define CHR_ERNGCHK              2
#define CHR_GFORMAT              4
#define CHR_DOLRSIGN             8
#define CHR_PLUSSIGN            16
#define CHR_TRAILZERO           32
#define CHR_TRAILDECP           64
#define CHR_COMMAFLG           128
#define CHR_TRAILBLNK          256
#define CHR_LEADZERO           512
#define CHR_EFMTPLUS          1024
#define CHR_MINUSPREN         2048
#define CHR_PADBLANK          4096
#define CHR_ASTERIX           8192
#define CHR_QULINES          16384
#define CHR_HASEXPON         32768  /* 16 */
#define CHR_ZDCOBOL          65536  /* 17 */
#define CHR_DOUBLE_QUOTE    131072  /* 18 */
#define CHR_SKIP_CRLF       262144  /* 19 */
#define CHR_SKIP_DELIM      524288  /* 20 */
#define CHR_LIST_NULL      1048576  /* 21 */
#define CHR_CPP_TEXT       2097152  /* 22 */
#define CHR_CASESENSITIVE  4194304  /* 23 */

#define CHR_DATE_HYPHEN          1
#define CHR_DATE_YEAR            2
#define CHR_DATE_DDMM            4
#define CHR_DATE_LEADZERO        8
#define CHR_DATE_MONTH          16
#define CHR_TIME_LEADZERO       32
#define CHR_TIME_AMPM           64
#define CHR_DATE_LEADBLANK     128
#define CHR_DATE_FULLNAME      256
#define CHR_DATE_YYMMDD        512

#define CHR_END_OF_RECORD        0
#define CHR_SPECIAL              1
#define CHR_IDENTIFIER           2
#define CHR_INTEGER              3
#define CHR_FLOAT                4
#define CHR_QUOTED               5
#define CHR_HEXCONS              6
#define CHR_DOUBLE               7
#define CHR_OCTAL                8
#define CHR_CHARACTER            9

#define CHR_BACK_ONLY            1
#define CHR_FRONT_ONLY           2

#define CHRERR_WRONGDEC          1
#define CHRERR_BADDIGITS         2
#define CHRERR_BADTIMESEP        3
#define CHRERR_MISSINGVALUE      4
#define CHRERR_BADMONTH          5
#define CHRERR_BADDAY            6
#define CHRERR_BADYEAR           7
#define CHRERR_OVERFLOW          8
#define CHRERR_DUPLICATE         9
#define CHRERR_BADTYPE          10
#define CHRERR_CLOSEDTYPE       11

   extern char  ChrAlphabet[];       /* Uppercase letters in sequence */
   extern int   ChrBufferSize;       /* Size character storage circular buffer */
   extern int   ChrCrossOver;        /* Year before crossing over to 1900 */
   extern int   ChrDateTime;         /* Character date time conversion flags */
   extern char  ChrDigits[];         /* Numeric digits in sequence */
   extern int   ChrEfmtchr;          /* E-format exponent character */
   extern int   ChrEfmtwth;          /* E-format field width */
   extern int   ChrEmantisa;         /* Width of in E-format mantissa */
   extern int   ChrEmaxfmt;          /* Upper threshhold for E-format */
   extern int   ChrEminfmt;          /* Lower threshhold for E-format */
   extern int   ChrEndComment;       /* Token end of record comment character */
   extern int   ChrError;            /* Character conversion error code */
   extern int   ChrFlags;            /* Numeric conversion control flags */
   extern const char* ChrFullDayNames[];   /* Full day names */
   extern const char* ChrFullNames[];      /* Full month names */
   extern int   ChrIhist;            /* Conversion error support information */
   extern UBYTE ChrInformation[];    /* Character information */
   extern int   ChrIsdble;           /* Indicates if double precision */
   extern UBYTE* ChrListEntry;       /* Points to last found list entry */
   extern UBYTE* ChrListInfo;        /* Points to last found list information */
   extern int   ChrLfill;            /* Left side numeric fill character */
   extern int   ChrModulo;           /* Character display modulus */
   extern char  ChrMonthNames[];     /* 3-character month names */
   extern int   ChrNdec;             /* Numeric conversion decimal places */
   extern int   ChrNsig;             /* Maximum number of significant digits */
   extern int   ChrNonsig;           /* Character to represent nonsig digits */
   extern int   ChrOverFlow;         /* Conversion overflow character */
   extern int   ChrPlus;             /* Plus sign character */
   extern int   ChrTen[];            /* The integer powers of ten */
   extern UBYTE ChrTypeFlags[];      /* Character type flags */
   extern int   ChrWidth;            /* Report column width */
   extern char  Chr3DayNames[];      /* 3-character day names */

#ifdef CHARTYPE
#define ChrIsLetter(x)  (ChrTypeFlags[x] & CLETTER)
#define ChrIsUpper(x)   (ChrTypeFlags[x] & CUPPERC)
#define ChrIsLower(x)   (ChrTypeFlags[x] & CLOWERC)
#define ChrIsIdent(x)   (ChrTypeFlags[x] & CIDENTC)
#define ChrIsDigit(x)   (ChrTypeFlags[x] & CNUMBER)
#define ChrIsWhite(x)   (ChrTypeFlags[x] & CWSPACE)
#define ChrIsSpecial(x) (ChrTypeFlags[x] & CSYMBOL)
#define ChrIsControl(x) (x < 32 || x > 127)
#endif

#ifdef FPROTOTYPE
   int      ChrAddToList(UBYTE* List,int nList,char* Symbol,int Length,UBYTE* Info,int nInfo);
   int      ChrCharToCode(char* String,int nString);
   int      ChrCharToDate(char* s,int nstr);
   double   ChrCharToDouble(char *str,int nstr);
   int      ChrCharToInteger(char** s,int blank);
   // int      ChrCodeToString(char* Dspdig,int Code);
   int      ChrCompare(char *s1,char *s2,int ns);
   int      ChrCompareStrings(char *s1,char *s2);
   void     ChrConvertCase(char* s,int ns,int upper);
   int      ChrDateString(int date,char* String);
   int      ChrDateTimeString(longlong datetime,char* String);
   longlong ChrDateTimeValue(char* String,int nString);
   int      ChrDateToChar(int DateValue,char* String);
   int      ChrDateToJulian(int day,int month,int year);
   int      ChrDateValue(char* String,int nString);
   int      ChrDateWidth(void);
   void     ChrDefineReserved(UBYTE* Words,UBYTE* Symbols);
   int      ChrDoubleDisplay(char* fiocrec,double value,int nsigdig);
   void     ChrDoubleToChar(double val,int ndig,int* pdecpt,int* psign, char* dspdig);
   int      ChrDoubleToString(char* dspdig,double val,int nsigdig,int cnvflags,int width,int ndec);
   int      ChrEditString(char* pattern,int nPattern,char* buffer,char* params,int patChar);
   int      ChrFindFirst(char* String,int nString,char* Substring);
   int      ChrFindIdent(UBYTE* StandardList,int Ident);
   int      ChrFindLast(char* String,char* Substring);
   UBYTE*   ChrFindListEntry(UBYTE* StandardList,int Index,int nInfo);
   char*    ChrFindParameter(char* params,int iPram,int* length);
   int      ChrFindToken(UBYTE* StandardList,char* Token,int Length);
   int      ChrFindSymbol(UBYTE* StandardList,char* Symbol);
   int      ChrGetIdentifier(UBYTE* Record);
   void     ChrGetDateAndTime(int* DateValue,int* TimeValue);
   int      ChrGetSymbol(char* Record);
   int      ChrGetToken(char** Position,UBYTE* Token);
   int      ChrHexString(ULONG Value,char* String,int base);
   int      ChrHexValue(char* String,int Length);
   void     ChrInitialize(void);
   char*    ChrIntegerToChar(int val);
   void     ChrJulianToDate(int Julian,int* Day,int* Month,int* Year);
   int      ChrLonglongToChar(longlong uval,char* dspdig,int length,char filc,int base);
   longlong ChrLonglongValue(char* String,int Length);
   longlong ChrLongValue(char* String,int nString,int nDecimal);
   int      ChrLongString(longlong Value,char* String,int nDecimal);
   int      ChrMoneyToChar(int MoneyValue,char* String);
   int      ChrOctalValue(char* String,int Length);
   int      ChrPicture(double value,char* Picture,char* crec);
   double   ChrPowerOfTen(int expon);
   int      ChrRoundValue(char* dspdig,int ndigit,int length);
   void     ChrShiftLeft(char* s, int n);
   void     ChrShiftRight(char* s, int n, int fill);
   int      ChrShortFromString(const char** s,int blank);
   int      ChrShortFromString(char** s,int blank);
   int      ChrShortString(int Value,char* String,int nDecimal);
   int      ChrShortValue(char* String,int nString,int nDecimal);
   char*    ChrStorage(int Length);
   int      ChrStringToCode(char* String);
   void     ChrSubstring(char* Substring,char* String,int iStart,int nSub);
   int      ChrTimeString(int TimeValue,int ShowSeconds,char* String);
   int      ChrTimeValue(char* String,int nString);
   int      ChrTimeWidth(void);
   char*    ChrTrimWhiteSpace(char* String,int Location);
   int      ChrUnsignedToChar(unsigned int uval,char* dspdig,int length,char filc,int base);
#else
   extern int      ChrAddToList();
   extern int      ChrCharToCode();       /* Compute Code Value of Bounded String */
   extern int      ChrCharToDate();
   extern double   ChrCharToDouble();     /* Convert characters to double */
   extern int      ChrCharToInteger();    /* Extract integer value from string */
   // extern int      ChrCodeToString();     /* Convert Code to String */
   extern int      ChrCompare();          /* Do a comparison between strings */
   extern int      ChrCompareStrings();   /* Compare null-terminated strings */
   extern void     ChrConvertCase();      /* Convert case to upper or lower */
   extern int      ChrDateString();       /* Convert Date Value to Date String */
   extern int      ChrDateTimeString();   /* Convert Date/Time Value to String */
   extern longlong ChrDateTimeValue();    /* Convert Date/Time String to Value */
   extern int      ChrDateToChar();
   extern int      ChrDateToJulian();     /* Convert Date to Julian Value */
   extern int      ChrDateValue();        /* Convert Date String to Date Value */
   extern int      ChrDateWidth();        /* Get Date Display Width */
   extern void     ChrDefineReserved();   /* Define Reserved Words and Symbols */
   extern int      ChrDoubleDisplay();    /* Display Double Precision Value */
   extern void     ChrDoubleToChar();     /* Convert Double to Raw Character Form */
   extern int      ChrDoubleToString();   /* Convert double value to string */
   extern int      ChrEditString();       /* Form string from edit pattern */
   extern int      ChrFindFirst();        /* Find first occurrence of substring */
   extern int      ChrFindIdent();
   extern int      ChrFindLast();         /* Find last occurrence of substring */
   extern UBYTE*   ChrFindListEntry();    /* Find list entry via index */
   extern char*    ChrFindParameter();    /* Find parameter in list */
   extern int      ChrFindToken();        /* Find Token in Standard List */
   extern int      ChrFindSymbol();
   extern void     ChrGetDateAndTime();   /* Get Current Date and Time */
   extern int      ChrGetIdentifier();
   extern int      ChrGetSymbol();        /* Get Next Symbol from Record */
   extern int      ChrGetToken();         /* Get Next Token from Record */
   extern int      ChrHexString();        /* Obtain hex string value from short value */
   extern int      ChrHexValue();         /* Obtain short value from hex string value */
   extern void     ChrInitialize();       /* Initialize Character Processing */
   extern char*    ChrIntegerToChar();    /* Convert integer to character */
   extern void     ChrJulianToDate();     /* Convert Julian Value to Date */
   extern int      ChrLonglongToChar();   /* Convert longlong to character */
   extern longlong ChrLonglongValue();    /* Obtain longlong value from string */
   extern longlong ChrLongValue();        /* Obtain Long Value from String Value */
   extern int      ChrLongString();       /* Obtain String Value from Long Value */
   extern int      ChrMoneyToChar();      /* Convert money to character form */
   extern int      ChrOctalValue();       /* Compute octal value of string */
   extern int      ChrPicture();
   extern double   ChrPowerOfTen();       /* Compute a power of 10 multiplier */
   extern int      ChrRoundValue();       /* Round display of value */
   extern void     ChrShiftLeft();        /* Shift character string left */
   extern void     ChrShiftRight();       /* Shift character string right */
   extern int      ChrShortFromString();  /* Extract Short Value from string */
   extern int      ChrShortString();      /* Obtain String Value from Short Value */
   extern int      ChrShortValue();       /* Obtain Short Value from String Value */
   extern char*    ChrStorage();          /* Get Character Storage Area */
   extern int      ChrStringToCode();     /* Compute the code value of a string */
   extern void     ChrSubstring();        /* Extract a substring of a string */
   extern int      ChrTimeString();       /* Convert Time Value to Time String */
   extern int      ChrTimeValue();        /* Convert Time String to Time Value */
   extern int      ChrTimeWidth();        /* Get Time Display Width */
   extern char*    ChrTrimWhiteSpace();   /* Trim white space from string */
   extern int      ChrUnsignedToChar();   /* Convert unsigned integer to string */
#endif


   // fifdtime
   /*
     ;/hdr/ ************************************************************************
     ;
     ; FifDtim.c: Fortran Date and Time Functions
     ;
     ;/hdr/ ************************************************************************
   */
#define DATETIME                  /* Gives access to date and time functions */
#define RAWMFUNC                  /* Gives access to raw memory functions */
   // #include "platform.h"             /* Define the platform hosting this code */
   // #include "pfclib.h"               /* Define Promula runtime library symbols */
   // #include "chrapi.h"               /* Define character manipulation interface */

#ifdef MPEPLAT
#else
#ifdef OSUNIX
#define HAS_GET_TIME
#endif
#endif

   /*
     ;/doc/ ***********************************************************************
     ;
     ; fifdate: FORTRAN Intrisic Function DATE
     ;
     ; Synopsis of Service:
     ;
     ; #include "fortran.h"                 pFortran runtime library interface
     ;
     ; void fifdate                         FORTRAN intrinsic function DATE
     ; (
     ;    char* cl                          Returns the date in string form
     ;    int   ncl                         Length of the date return string
     ; )
     ;
     ; Description of Service:
     ;
     ; This service obtains the current date from the operating system in numeric
     ; form and converts it to the desired display form.
     ;
     ; Properties of Service:
     ;
     ; cl           Returns the date in string form
     ;
     ; ncl          Length of the date return string
     ;
     ; Return Value from Service: None
     ;
     ;/doc/ ************************************************************************
   */
#ifdef FPROTOTYPE
   void fifdate(char* cl,int ncl)
#else
      void fifdate(cl,ncl)
      char* cl;                         /* Returns the date in string form */
   int ncl;                          /* Length of the date return string */
#endif
   {
      char date[10];
      int ival;
      char* s1;
      struct tm *newtime;
      time_t longtime;
      static char mnth[ ] = 
         {
            'J','A','N',
            'F','E','B',
            'M','A','R',
            'A','P','R',
            'M','A','Y',
            'J','U','N',
            'J','U','L',
            'A','U','G',
            'S','E','P',
            'O','C','T',
            'N','O','V',
            'D','E','C'
         };
      time(&longtime);
      newtime = localtime(&longtime); 
      if(fioconv & ZDVAXFT) 
      {
         date[2] = date[6] = '-';
         ival = newtime->tm_mday;
         date[0] = ChrDigits[ival / 10];
         if(date[0] == '0') date[0] = ' ';
         date[1] = ChrDigits[ival % 10];
         s1 = mnth + newtime->tm_mon * 3;
         date[3] = *s1;
         date[4] = *(s1+1);
         date[5] = *(s1+2);
         ival = (newtime->tm_year) % 100;
         date[7] = ChrDigits[ival / 10];
         date[8] = ChrDigits[ival % 10];
         ival = 9;
      }
      else 
      {
         date[2] = date[5] = '/';
         ival = newtime->tm_mon+1;
         date[0] = ChrDigits[ival / 10];
         date[1] = ChrDigits[ival % 10];
         ival = newtime->tm_mday;
         date[3] = ChrDigits[ival / 10];
         date[4] = ChrDigits[ival % 10];
         ival = (newtime->tm_year) % 100;
         date[6] = ChrDigits[ival / 10];
         date[7] = ChrDigits[ival % 10];
         ival = 8;
      }
      filmem(cl,ncl,' ');
      if(ival > ncl) ival = ncl;
      cpymem(date,cl,ival);
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; fifi2dat: FORTRAN Intrinsic Function I2DATE
     ;
     ; Synopsis of Service:
     ;
     ; #include "fortran.h"                 pFortran runtime library interface
     ;
     ; void fifi2date                       FORTRAN intrinsic function I2DATE
     ; (
     ;    short* mm                         Returns month value
     ;    short* dd                         Returns day value
     ;    short* yy                         Returns year value
     ; )
     ;
     ; Description of Service:
     ;
     ; This service obtains the current date from the operating system and returns
     ; the month, day, and year in the properties. Note that it has a hostile user
     ; assumption which checks that none of the properties are NULL.
     ;
     ; Properties of Service:
     ;
     ; mm           Returns month value
     ;
     ; dd           Returns day value
     ;
     ; yy           Returns year value
     ;
     ; Return Value from Service: None
     ;
     ;/doc/ ************************************************************************
   */
#ifdef FPROTOTYPE
   void fifi2date(short* mm,short* dd,short* yy)
#else
      void fifi2date(mm,dd,yy)
      short* mm;                     /* Returns month value */
   short* dd;                     /* Returns day value */
   short* yy;                     /* Returns year value */
#endif
   {
      struct tm *newtime;
      time_t longtime;

      time(&longtime);
      newtime = localtime(&longtime); 
#ifdef ISUNALIGNED
      if(mm != NULL) put16s((char*)(mm),(short)(newtime->tm_mon+1));
      if(dd != NULL) put16s((char*)(dd),(short)(newtime->tm_mday));
      if(yy != NULL) put16s((char*)(yy),(short)(newtime->tm_year));
#else
      if(mm != NULL) *mm = (short)(newtime->tm_mon+1);
      if(dd != NULL) *dd = (short)(newtime->tm_mday);
      if(yy != NULL) *yy = (short)(newtime->tm_year);
#endif
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; fifidate: FORTRAN Intrinsic Function IDATE
     ;
     ; Synopsis of Service:
     ;
     ; #include "fortran.h"                 pFortran runtime library interface
     ;
     ; void fifidate                        FORTRAN Intrisic function IDATE
     ; (
     ;    LONG* mm                          Returns month value
     ;    LONG* dd                          Returns day value
     ;    LONG* yy                          Returns year value
     ; )
     ;
     ; Description of Service:
     ;
     ; This service obtains the date from the operating system in numeric form and
     ; returns the month, day, and year in its properties. Note that it has a
     ; hostile user assumption which checks that none of the properities are NULL.
     ;
     ; Properties of Service:
     ;
     ; mm           Returns month value
     ;
     ; dd           Returns day value
     ;
     ; yy           Returns year value
     ;
     ; Return Value from Service: None
     ;
     ;/doc/ ************************************************************************
   */
#ifdef FPROTOTYPE
   void fifidate(LONG* mm,LONG* dd,LONG* yy)
#else
      void fifidate(mm,dd,yy)
      LONG* mm;                      /* Returns month value */
   LONG* dd;                      /* Returns day value */
   LONG* yy;                      /* Returns year value */
#endif
   {
      struct tm *newtime;
      time_t longtime;

      time(&longtime);
      newtime = localtime(&longtime); 
#ifdef ISUNALIGNED
      if(mm != NULL) put32s((char*)(mm),(LONG)(newtime->tm_mon+1));
      if(dd != NULL) put32s((char*)(dd),(LONG)(newtime->tm_mday));
      if(yy != NULL) put32s((char*)(yy),(LONG)(newtime->tm_year));
#else
      if(mm != NULL) *mm = newtime->tm_mon+1;
      if(dd != NULL) *dd = newtime->tm_mday;
      if(yy != NULL) *yy = newtime->tm_year;
#endif
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; fifitime: FORTRAN Intrinsic Function ITIME
     ;
     ; Synopsis of Service:
     ;
     ; #include "fortran.h"                 pFortran runtime library interface
     ;
     ; void fifitime                        FORTRAN intrinsic function ITIME
     ; (
     ;    LONG* hour                        Returns the current hour
     ;    LONG* min                         Returns the current minute
     ;    LONG* sec                         Returns the current second
     ; )
     ;
     ; Description of Service:
     ;
     ; This service obtains the current time from the operating system in numeric
     ; form and returns the current time of day in hours, minutes, and seconds in
     ; its three properties. Note that it has a hostile user assumption which checks
     ; that none of the properties are NULL.
     ;
     ; Properties of Service:
     ;
     ; hour         Returns the current hour
     ;
     ; min          Returns the current minute
     ;
     ; sec          Returns the current second
     ;
     ; Return Value from Service: None
     ;
     ;/doc/ ************************************************************************
   */
#ifdef FPROTOTYPE
   void fifitime(LONG* hour,LONG* min,LONG* sec)
#else
      void fifitime(hour,min,sec)
      LONG* hour;                    /* Returns the current hour */
   LONG* min;                     /* Returns the current minute */
   LONG* sec;                     /* Returns the current second */
#endif
   {
      struct tm *newtime;
      time_t longtime;

      time(&longtime);
      newtime = localtime(&longtime); 
      if(hour != NULL) *hour = newtime->tm_hour;
      if(min != NULL) *min = newtime->tm_min;
      if(sec != NULL) *sec = newtime->tm_sec;
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; fifsecnd: FORTRAN Intrinsic Function SECNDS
     ;
     ; Synopsis of Service:
     ;
     ; #include "fortran.h"                 pFortran runtime library interface
     ;
     ; float fifsecnd                       FORTRAN intrinsic function SECNDS
     ; (
     ;    float y                           User-supplied previous time value
     ; )
     ;
     ; Description of Service:
     ;
     ; This service returns the system time in seconds as a float value minus the
     ; value of its argument.
     ;
     ; Properties of Service:
     ;
     ; y            User-supplied previous time value
     ;
     ; Return Value from Service:
     ;
     ; The float value as described above.
     ;
     ;/doc/ ************************************************************************
   */
#ifdef FPROTOTYPE
   float fifsecnd(float y)
#else
      float fifsecnd(y)
      float y;                       /* User-supplied value */
#endif
   {
#ifdef HAS_GET_TIME

      timeval tv;               /* tv[0]=seconds tv[1]=microseconds */

#else

      struct tm *newtime;
      time_t longtime;

#endif

      double systim;

#ifdef HAS_GET_TIME

      gettimeofday(&tv,0);            /* get current system time */
      tv.tv_sec %= 86400;                /* modulo by one day */
      systim = (double)(tv.tv_sec) + (double)(tv.tv_usec) * 1.e-6;

#else

      time(&longtime);
      newtime = localtime(&longtime); 
      systim = ((double)newtime->tm_hour) * 3600.0 +
         ((double)newtime->tm_min)  *  60.0 +
         ((double)newtime->tm_sec);
#endif

      if(systim <= y) systim += y;
      else systim -= y;
      return (float)(systim);
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; fiftime: FORTRAN Intrinsic Function TIME
     ;
     ; Synopsis of Service:
     ;
     ; #include "fortran.h"                 pFortran runtime library interface
     ;
     ; void fiftime                         FORTRAN intrinsic function TIME
     ; (
     ;    char* cl                          Returns the time in string form
     ;    int  ncl                          Length of the time return string
     ; )
     ;
     ; Description of Service:
     ;
     ; This service obtain the time from the operating system in numeric form and
     ; converts it to the desired display form of hr:mi:sec .
     ;
     ; Properties of Service:
     ;
     ; cl           Returns the time in string form
     ;
     ; ncl          Length of the time return string
     ;
     ; Return Value from Service: None
     ;
     ;/doc/ ************************************************************************
   */
#ifdef FPROTOTYPE
   void fiftime(char* cl,int ncl)
#else
      void fiftime(cl,ncl)
      char* cl;                         /* Returns the time in string form */
   int ncl;                          /* Length of the time return string */
#endif
   {
      char timev[8];
      int ival;
      struct tm *newtime;
      time_t longtime;

      time(&longtime);
      newtime = localtime(&longtime); 
      ival = newtime->tm_hour;
      timev[0] = ChrDigits[ival / 10];
      timev[1] = ChrDigits[ival % 10];
      timev[2] = ':';
      ival = newtime->tm_min;
      timev[3] = ChrDigits[ival / 10];
      timev[4] = ChrDigits[ival % 10];
      timev[5] = ':';
      ival = newtime->tm_sec;
      timev[6] = ChrDigits[ival / 10];
      timev[7] = ChrDigits[ival % 10];
      filmem(cl,ncl,' ');
      ival = ncl;
      if(ival > 8) ival = 8;
      cpymem(timev,cl,ival);
   }

   // fifreal

   /*
     ;/hdr/ ************************************************************************
     ;
     ; Copyright 1986-1998 Promula Development Corporation ALL RIGHTS RESERVED
     ;
     ; name of module: FifReal.c -- Fortran Real Arithmetic
     ;
     ; global symbols defined:
     ;
     ; useful notes and assumptions:
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;         author: (LCC) Lois Carver, Promula Development Corporation
     ;
     ; edit 08/04/98 LCC: Converted headers and comments to new standard.
     ;
     ; edit 06/17/98 FKG: Changed names of global symbols to conform to the 
     ;                    Promula version 7.08 API naming conventions. 
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
     ;                explicit support for real arithmetic
     ;
     ;/hdr/ ************************************************************************
   */
#define MATHFUNC                  /* Gives access to mathematical functions */
   // #include "platform.h"             /* Define the platform hosting this code */
   // #include "pfclib.h"               /* Define PROMULA.FORTRAN library symbols */

   /*
     ;/doc/ ***********************************************************************
     ;
     ; name of routine: FIFSNCS -- FORTRAN Single Precision Sine/Cosine
     ;
     ; Computes the sine or cosine of a single precision value.
     ;
     ; calling parameters:
     ;
     ; return parameters:
     ;
     ; The value as computed above.
     ;
     ; useful notes and assumptions:
     ;
     ; Element        Description of use
     ; -------        ------------------
     ; fifsin         Compute single precision sine value
     ; fifcos         Compute single precision cosine value
     ;
     ; Compute the value using the standard series expansion but with all
     ; coefficients restricted to a single precision range.
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
     ;                explicit support for real arithmetic
     ;
     ;/doc/ ************************************************************************
   */

#ifdef FPROTOTYPE
   float fifsncs(float x,int cosflag)
#else
      float fifsncs(x,cosflag)
      float x;                          /* Argument containing the value */
   int cosflag;                      /* Perform cosine flag */

#endif
   {
#ifdef ANSPLAT
#define zero   0.0F
#else
#define zero   0.0
#endif
      static double r[8]=
         {
            -0.166667e0,
            0.833333e-2,
            -0.198413e-3,
            0.275573e-5,
            -0.250521e-7,
            0.160589e-9,
            -0.764292e-12,
            0.272048e-14
         };
      double y;
      int sgn;
      int j;
      int n;
      double f;
      double g;
      double rg;
#ifdef LONGDBL
      long double xn;
#else
      double xn;
#endif

      if(cosflag) 
      {
         y = fabs(x) + 1.57079;
         sgn = 0;
      }
      else if(x < 0.0) 
      {
         sgn = 1;
         y = -x;
      }
      else 
      {
         sgn = 0;
         y = x;
      }
      if(y >= 1.0e8) return zero;
      if(modf(y*0.31831,&xn) >= 0.5) xn += 1.0;
      n = (int) xn;
      if(n & 1) sgn =! sgn;
      if(cosflag) xn -= 0.5;
      f = fabs(x)-3.14160138*xn;
      f += 8.9089102e-6*xn;
      if(fabs(f) > 1.0e-8)
      {
         g = f*f;
         for(rg = 0.0,j=8; j--;) rg = (rg+r[j])*g;
         f = f+f*rg;
      }
      if(sgn) f = -f;
      return (float) f;
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; name of routine: FIFACOSD -- FORTRAN Intrinsic Function ACOSD
     ;
     ; Computes the arccosine in degrees of a floating point argument using
     ; single precision.
     ;
     ; calling parameters:
     ;
     ; return parameters:
     ;
     ; The value as computed above.
     ;
     ; useful notes and assumptions:
     ;
     ; Element        Description of use
     ; -------        ------------------
     ; FORTRAN        Intrinsic function COSD with a float argument
     ;
     ; Compute the value using the standard arccosine function.
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
     ;                explicit support for real arithmetic
     ;
     ;/doc/ ************************************************************************
   */

#ifdef FPROTOTYPE
   float fifacosd(float x)
#else
      float fifacosd(x)
      float x;                          /* Argument containing the value */
#endif
   {
      float ans;

      ans = (float)(acos((double) x)*180.0/3.141592654);
      return ans;
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; name of routine: FIFAMAX0 -- FORTRAN Intrinsic Function AMAX0
     ;
     ; Determines which of two long integer values is the largest.
     ;
     ; calling parameters:
     ;
     ; return parameters:
     ;
     ; The largest long integer value converted to floating point.
     ;
     ; useful notes and assumptions:
     ;
     ; Element        Description of use
     ; -------        ------------------
     ; FORTRAN        Intrinsic function AMAX0
     ;
     ; Compare the two values and return the largest.
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
     ;                explicit support for real arithmetic
     ;
     ;/doc/ ************************************************************************
   */

#ifdef FPROTOTYPE
   float fifamax0(LONG a1,LONG a2)
#else
      float fifamax0(a1,a2)
      LONG a1;                          /* First value to be compared */
   LONG a2;                          /* Second value to be compared */
#endif
   {
      if(a1 >= a2) return (float) a1;
      else return (float) a2;
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; name of routine: FIFAMIN0 -- FORTRAN Intrinsic Function AMIN0
     ;
     ; Determines which of two long integer values is the smallest.
     ;
     ; calling parameters:
     ;
     ; return parameters:
     ;
     ; The smallest long integer value converted to floating point.
     ;
     ; useful notes and assumptions:
     ;
     ; Element        Description of use
     ; -------        ------------------
     ; FORTRAN        Intrinsic function AMIN0
     ;
     ; Compare the two values and return the smallest.
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
     ;                explicit support for real arithmetic
     ;
     ;/doc/ ************************************************************************
   */

#ifdef FPROTOTYPE
   float fifamin0(LONG a1,LONG a2)
#else
      float fifamin0(a1,a2)
      LONG a1;                          /* First value to be compared */
   LONG a2;                          /* Second value to be compared */
#endif
   {
      if(a1 <= a2) return (float) a1;
      else return (float) a2;
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; name of routine: FIFASIND -- FORTRAN Intrinsic Function ASIND
     ;
     ; Computes the arcsine in degrees of a floating point argument using single
     ;
     ; calling parameters:
     ;
     ; return parameters:
     ;
     ; The value as computed above.
     ;
     ; useful notes and assumptions:
     ;
     ; Element        Description of use
     ; -------        ------------------
     ; FORTRAN        Intrinsic function SIND with a float argument
     ;
     ; Compute the value using the standard arcsine function.
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
     ;                explicit support for real arithmetic
     ;
     ;/doc/ ************************************************************************
   */

#ifdef FPROTOTYPE
   float fifasind(float x)
#else
      float fifasind(x)
      float x;                          /* Argument containing the value */
#endif
   {
      float ans;

      ans = (float)(asin((double) x)*180.0/3.141592654);
      return ans;
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; name of routine: FIFATAND -- FORTRAN Intrinsic Function ATAND
     ;
     ; Computes the arctangent in degrees of a floating point argument using
     ; single precision.
     ;
     ; calling parameters:
     ;
     ; return parameters:
     ;
     ; The value as computed above.
     ;
     ; useful notes and assumptions:
     ;
     ; Element        Description of use
     ; -------        ------------------
     ; FORTRAN        Intrinsic function TAND with a float argument
     ;
     ; Compute the value using the standard arctangent function.
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
     ;                explicit support for real arithmetic
     ;
     ;/doc/ ************************************************************************
   */

#ifdef FPROTOTYPE
   float fifatand(float x)
#else
      float fifatand(x)
      float x;                          /* Argument containing the value */
#endif
   {
      float ans;

      ans = (float)(atan((double) x)*180.0/3.141592654);
      return ans;
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; name of routine: FIFCOS -- FORTRAN Intrinsic Function COS
     ;
     ; Computes the cosine of a floating point argument using single precision.
     ;
     ; calling parameters:
     ;
     ; return parameters:
     ;
     ; The double precision value as computed above.
     ;
     ; useful notes and assumptions:
     ;
     ; Element        Description of use
     ; -------        ------------------
     ; FORTRAN        Intrinsic function SIN
     ;
     ; Simply use the sine-cosine function to compute the result.
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
     ;                explicit support for real arithmetic
     ;
     ;/doc/ ************************************************************************
   */

#ifdef FPROTOTYPE
   float fifcos(float x)
#else
      float fifcos(x)
      float x;                          /* Argument containing the value */
#endif
   {
      return fifsncs(x,1);
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; name of routine: FIFCOSD -- FORTRAN Intrinsic Function COSD
     ;
     ; Computes the cosine of a floating point argument in degrees using single
     ; precision.
     ;
     ; calling parameters:
     ;
     ; return parameters:
     ;
     ; The value as computed above.
     ;
     ; useful notes and assumptions:
     ;
     ; Element        Description of use
     ; -------        ------------------
     ; FORTRAN        Intrinsic function COSD with a float argument
     ;
     ; Compute the value using the standard cosine function.
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
     ;                explicit support for real arithmetic
     ;
     ;/doc/ ************************************************************************
   */

#ifdef FPROTOTYPE
   float fifcosd(float x)
#else
      float fifcosd(x)
      float x;                          /* Argument containing the value */
#endif
   {
      float ans;

      ans = (float)cos((double)x * 3.141592654/180.0);
      return ans;
   }

#ifdef FPROTOTYPE
   double fifdcosd(double x)
#else
      double fifdcosd(x)
      double x;
#endif
   {
      double ans;
      ans=cos(x*3.141592654/180.0);
      return ans;
   }
   /*
     ;/doc/ ***********************************************************************
     ;
     ; name of routine: FIFDDIM -- FORTRAN Intrinsic Function DDIM
     ;
     ; Computes a positive difference.
     ;
     ; calling parameters:
     ;
     ; return parameters:
     ;
     ; If a1 is greater than a2 then the value of a1 - a2 is returned, else zero
     ; is returned.
     ;
     ; useful notes and assumptions:
     ;
     ; Element        Description of use
     ; -------        ------------------
     ; FORTRAN        Intrinsic function DDIM and generic function DIM
     ;                when used with float or double arguments.
     ;
     ; Do the comparison of a1 and a2 and return the value specified above.
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
     ;                explicit support for real arithmetic
     ;
     ;/doc/ ************************************************************************
   */

#ifdef FPROTOTYPE
   double fifddim(double a1,double a2)
#else
      double fifddim(a1,a2)
      double a1;                        /* First value in difference */
   double a2;                        /* Second value in difference */
#endif
   {
      if(a1 > a2) return a1 - a2;
      else return 0.0;
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; name of routine: FIFDINT -- FORTRAN Intrinsic Function DINT
     ;
     ; Truncates its double precision argument to its integer value.
     ; In particular:
     ;
     ;      fifdint(a) = 0 if |a| < 1
     ;                 = the largest integer with the same sign as a that
     ;                   does not exceed a if |a| >= 1
     ;
     ; calling parameters:
     ;
     ; return parameters:
     ;
     ; The double precision value as computed above.
     ;
     ; useful notes and assumptions:
     ;
     ; Element        Description of use
     ; -------        ------------------
     ; FORTRAN        Intrinsic function DINT and the generic function AINT
     ;
     ; This value is simply the integal return value from the C "modf" function
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
     ;                explicit support for real arithmetic
     ;
     ;/doc/ ************************************************************************
   */

#ifdef FPROTOTYPE
   double fifdint(double a)
#else
      double fifdint(a)
      double a;                         /* Argument containing the value */
#endif
   {
#ifdef LONGDBL
      long double temp;
      modf(a,&temp);
      a = temp;
#else
      modf(a,&a);
#endif
      return a;
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; name of routine: FIFDMAX1 -- FORTRAN Intrinsic Function DMAX1
     ;
     ; Determines which of two double precision values is the largest.
     ;
     ; calling parameters:
     ;
     ; return parameters:
     ;
     ; The largest double precision value.
     ;
     ; useful notes and assumptions:
     ;
     ; Element        Description of use
     ; -------        ------------------
     ; FORTRAN        Intrinsic functions AMAX0, AMAX1, and DMAX1
     ;
     ; Compare the two values and return the largest.
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
     ;                explicit support for real arithmetic
     ;
     ;/doc/ ************************************************************************
   */

#ifdef FPROTOTYPE
   double fifdmax1(double a1,double a2)
#else
      double fifdmax1(a1,a2)
      double a1;                        /* First value to be compared */
   double a2;                        /* Second value to be compared */
#endif
   {
      if(a1 >= a2) return a1;
      else return a2;
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; name of routine: FIFDMIN1 -- FORTRAN Intrinsic Function DMIN1
     ;
     ; Determines which of two double precision values is the smallest.
     ;
     ; calling parameters:
     ;
     ; return parameters:
     ; 
     ; The smallest double precision value.
     ;
     ; useful notes and assumptions:
     ;
     ; Element        Description of use
     ; -------        ------------------
     ; FORTRAN        Intrinsic functions AMIN1 and DMIN1
     ;
     ; Compare the two values and return the smallest.
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
     ;                explicit support for real arithmetic
     ;
     ;/doc/ ************************************************************************
   */

#ifdef FPROTOTYPE
   double fifdmin1(double a1,double a2)
#else
      double fifdmin1(a1,a2)
      double a1;                        /* First value to be compared */
   double a2;                        /* Second value to be compared */
#endif
   {
      if(a1 <= a2) return a1;
      else return a2;
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; name of routine: FIFDMOD -- FORTRAN Intrinsic Function DMOD
     ;
     ; Computes the value of the remainder of num divided by dem. If dem is zero,
     ; then the result is zero. For nonzero values the result is calculated as
     ; follows:
     ;
     ;     num - (floor(num / dem) * dem)
     ;
     ; where floor is the standard C "floor" function.
     ;
     ; calling parameters:
     ;
     ; return parameters:
     ;
     ; The value as computed above.
     ;
     ; useful notes and assumptions:
     ;
     ; Element        Description of use
     ; -------        ------------------
     ; FORTRAN        Intrinsic functions AMOD and DMOD and generic function MOD
     ;                when used with float or double arguments.
     ;
     ; This value is computed as above if dem is nonzero, else it is zero.
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
     ;                explicit support for real arithmetic
     ;
     ;/doc/ ************************************************************************
   */

#ifdef FPROTOTYPE
   double fifdmod(double num,double dem)
#else
      double fifdmod(num,dem)
      double num;                       /* The numerator for the calculation */
   double dem;                       /* The denominator for the calculation */
#endif
   {
      int sign;

      if(dem == 0.0) return 0.0;
      if(num < 0) 
      {
         sign = 1;
         num = -num;
      }
      else sign = 0;
      if(dem < 0) dem = -dem;
      num -= floor(num/dem)*dem;
      if(sign) num = -num;
      return num;
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; name of routine: FIFDNINT -- FORTRAN Intrinsic Function DNINT
     ;
     ; Computes the nearest integer to its double precision argument.
     ; In particular:
     ;
     ;      fifdnint(a) = fifdint(a+0.5) if a >= 0.0
     ;                  = fifdint(a-0.5) if a <= 0.0
     ;
     ; where "fifdint" is the FORTRAN intrinsic function DINT.
     ;
     ; calling parameters:
     ;
     ; return parameters:
     ;
     ; The value as computed above.
     ;
     ; useful notes and assumptions:
     ;
     ; Element        Description of use
     ; -------        ------------------
     ; FORTRAN        Intrinsic functions ANINT and DNINT
     ;
     ; Simply do the calculation as specified above.
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
     ;                explicit support for real arithmetic
     ;
     ;/doc/ ************************************************************************
   */

#ifdef FPROTOTYPE
   double fifdnint(double a)
#else
      double fifdnint(a)
      double a;                         /* Argument containing the value */
#endif
   {
      if(a < 0.0) a -= 0.5;
      else a += 0.5;
      return fifdint(a);
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; name of routine: FIFDSIGN -- FORTRAN Intrinsic Function DSIGN
     ;
     ; Returns a value after the transfer of a sign. The result is |mag| if sgn
     ; is at least zero, else it is -|mag|.
     ;
     ; calling parameters:
     ;
     ; return parameters:
     ;
     ; The value as computed above.
     ;
     ; useful notes and assumptions:
     ;
     ; Element        Description of use
     ; -------        ------------------
     ; FORTRAN        Intrinsic function DSIGN and generic function SIGN
     ;                when used with float or double arguments.
     ;
     ; If the magnitude and the sign do not have the same sign, return the negative
     ; of the magnitude; else simply return the magnitude.
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
     ;                explicit support for real arithmetic
     ;
     ;/doc/ ************************************************************************
   */

#ifdef FPROTOTYPE
   double fifdsign(double mag,double sgn)
#else
      double fifdsign(mag,sgn)
      double mag;                       /* The magnitude for the result */
   double sgn;                       /* The sign for the result */
#endif
   {
      if((mag > 0.0 && sgn < 0.0) || (mag < 0.0 && sgn >= 0.0)) return -mag;
      else return mag;
   }

#ifdef FPROTOTYPE
   double fifdsind(double x)
#else
      double fifdsind(x)
      double x;
#endif
   {
      double ans;
      ans=sin(x*3.141592654/180.0);
      return ans;
   }

#ifdef FPROTOTYPE
   double fifdtand(double x)
#else
      double fifdtand(x)
      double x;
#endif
   {
      double ans;
      ans=tan(x*3.141592654/180.0);
      return ans;
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; name of routine: FIFEXP -- FORTRAN Intrinsic Function EXP
     ;
     ; Computes the exponential of a floating point argument using single
     ; precision.
     ;
     ; calling parameters:
     ;
     ; return parameters:
     ;
     ; The value as computed above.
     ;
     ; useful notes and assumptions:
     ;
     ; Element        Description of use
     ; -------        ------------------
     ; FORTRAN        Intrinsic function EXP with a float argument
     ;
     ; Compute the value using a standard approximation.
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
     ;                explicit support for real arithmetic
     ;
     ;/doc/ ************************************************************************
   */

#ifdef FPROTOTYPE
   float fifexp(float x)
#else
      float fifexp(x)
      float x;                          /* Argument containing the value */
#endif
   {
#ifdef ANSPLAT
#define bigval  500.0F
#define coef1   1.44269504088896F
#define coef2   0.693359375F
#define coef3   2.12194440054690e-4F
#define coef4   0.165203300268279e-4F
#define coef5   0.694360001511792e-2F
#define coef6   0.24999999999992F
#define coef7   0.495862884905441e-3F
#define coef8   0.555538666969001e-1F
#define point5  0.5F
#define three   3.0F
#define one     1.0F
#define zero    0.0F
#else
#define bigval  500.0
#define coef1   1.44269504088896
#define coef2   0.693359375
#define coef3   2.12194440054690e-4
#define coef4   0.165203300268279e-4
#define coef5   0.694360001511792e-2
#define coef6   0.24999999999992
#define coef7   0.495862884905441e-3
#define coef8   0.555538666969001e-1
#define point5  0.5
#define three   3.0
#define one     1.0
#define zero    0.0
#endif
      int n;
      float temp;
      float xn;
      float g;
      float z;
      float gpz;
      float qz;
      float rg;

      if(x == zero) return one;
      if(x < -bigval || x > bigval) return zero;
      temp = (float)(x * coef1);
      xn = (float) floor(temp);
      if((temp - xn) > point5) xn += one;
      n = (int) xn;
      g = (float)(x - xn * coef2);
      g = (float)(g + (xn * coef3));
      z = g * g;
      gpz = (float)(((coef4 * z + coef5) * z + coef6) * g);
      qz = (float)((coef7 - three * z + coef8) * z + point5);
      rg = (float)(point5 + gpz / (qz - gpz));
      return (float) ldexp(rg, n+1);

#undef coef1
#undef coef2
#undef coef3
#undef coef4
#undef coef5
#undef coef6
#undef coef7
#undef coef8
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; name of routine: FIFFRAC -- FORTRAN Intrinsic Function FRAC
     ;
     ; Returns the fractional part of a real number.
     ;
     ; calling parameters:
     ;
     ; return parameters:
     ; 
     ; The double precision value as computed above.
     ;
     ; useful notes and assumptions:
     ;
     ; Element        Description of use
     ; -------        ------------------
     ; FORTRAN        Intrinsic function DFREAC and the generic function FRAC
     ;
     ; This value is simply the fractional return value from C "modf" function
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
     ;                explicit support for real arithmetic
     ;
     ;/doc/ ************************************************************************
   */

#ifdef FPROTOTYPE
   double fiffrac(double a)
#else
      double fiffrac(a)
      double a;                         /* Argument containing the value */
#endif
   {
      double temp;

      return modf(a,&temp);
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; name of routine: FIFLOG -- FORTRAN Intrinsic Function LOG
     ;
     ; Computes the natural logarithm of a floating point argument using single
     ; precision.
     ;
     ; calling parameters:
     ;
     ; return parameters:
     ;
     ; The value as computed above.
     ;
     ; useful notes and assumptions:
     ;
     ; Element        Description of use
     ; -------        ------------------
     ; FORTRAN        Intrinsic function LOG with a float argument
     ;
     ; Compute the value using a standard approximation.
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
     ;                explicit support for real arithmetic
     ;
     ;/doc/ ************************************************************************
   */

#ifdef FPROTOTYPE
   float fiflog(float val)
#else
      float fiflog(val)
      float val;                        /* Argument containing the value */
#endif
   {
      int n;
      float f;
      float znum;
      float zden;
      float z;
      float w;
      float aw;
      float bw;
      float rz;
#ifdef ANSPLAT
#define zero    0.0F
#define point5  0.5F
#define coef1   0.707106781186547F
#define coef2   0.356679777390346e2F
#define coef3   0.312032220919245e3F
#define coef4   0.769499321084948e3F
#define coef5   -0.789561128874912e0F
#define coef6   0.163839435630215e2F
#define coef7   0.641249434237455e2F
#define coef8   2.1219444005469e-4F
#define v355    355.0F
#else
#define zero    0.0
#define point5  0.5
#define coef1   0.707106781186547
#define coef2   0.356679777390346e2
#define coef3   0.312032220919245e3
#define coef4   0.769499321084948e3
#define coef5   -0.789561128874912e0
#define coef6   0.163839435630215e2
#define coef7   0.641249434237455e2
#define coef8   2.1219444005469e-4
#define v355    355.0
#endif

      if(val <= zero) return zero;
      f = (float) frexp(val, &n);
      znum = (float)(f - point5);
      if(f > coef1) 
      {
         znum -= point5;
         zden = (float)(f * point5 + point5);
      }
      else
      {
         n -= 1;
         zden = (float)(znum * point5 + point5);
      }
      z = znum / zden;
      w = z * z;
      bw = (float)(((w - coef2) * w + coef3) * w - coef4);
      aw = (float)((coef5 *w + coef6) * w - coef7);
      rz = z + z * (w * aw / bw);
      val = v355;
      val /= 512;
      val *= n;
      val = (float)(val + rz - n * coef8);
      return (float)(val);
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; name of routine: FIFPOW -- FORTRAN Intrinsic Function POW
     ;
     ; Raises a float value to a float power.
     ;
     ; calling parameters:
     ;
     ; return parameters:
     ;
     ; The float result.
     ;
     ; useful notes and assumptions:
     ;
     ; Element        Description of use
     ; -------        ------------------
     ; FORTRAN        The ** operator when applied to floats
     ;
     ; Simply use the standard exp and log functions to compute the result for
     ; nonintegral exponents. For integral powers, the function builds a table of
     ; the value raised to the various powers of two. It then combines these
     ; powers, using the fact that any number can be expressed as the sum of
     ; powers of 2. Note that if the value it raised is negative, then the power
     ; must be integral.
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
     ;                explicit support for real arithmetic
     ;
     ;/doc/ ************************************************************************
   */

#ifdef FPROTOTYPE
   float fifpow(float x,float y)
#else
      float fifpow(x,y)
      float x;                          /* Value to be raised to a power */
   float y;                          /* Power to be used */
#endif
   {
#ifdef ANSPLAT
#define zero  0.0F
#define one   1.0F
#else
#define zero  0.0
#define one   1.0
#endif
      int expon;
      int sign;
      int i;
      int p2;
      int havtab;
      float temp;
      float power2[6];

      expon = (int) y;
      if((float)expon == y) 
      {
         if(expon == 0) return one;
         if(expon < 0) 
         {
            sign = 1;
            expon = -expon;
         }
         else sign = 0;
         if((expon) > 9999) return zero;
         temp = (float)(x);
         havtab = 0;
         p2 = 32;
         for(i = 0, p2 = 32; i < 6 && expon > 0; i++, p2 /= 2)
         {
            while(expon > p2) 
            {
               if(!havtab) 
               {
                  power2[5] = (float)(x);
                  for(havtab = 5; havtab > i; havtab--)
                     power2[havtab-1] = power2[havtab] * power2[havtab];
                  havtab = 1;
               }
               temp *= power2[i];
               expon -= p2;
            }
         }
         if(sign) temp = (float)(one / temp);
      }
      else 
      {
         if(x <= zero) return zero;
         temp = fifexp(y * fiflog(x));
      }
      return temp;
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; name of routine: FIFRAN -- FORTRAN Intrinsic Function RAN
     ;
     ; Is a general random number generator of the multiplicative congruential
     ; type. The result is a floating-point number that is uniformly distributed
     ; in the range between 0.0 and 1.0 exclusively. The agument should initially
     ; be set to a large, odd integer value. This function updates that value
     ; using the following computation, which typically causes an overflow:
     ;
     ;      seed = 69069 * seed + 1
     ;
     ; The value returned is the upper 24 bits of the updated seed converted to
     ; floating point and divided by 2 ** 24.
     ;
     ; calling parameters:
     ;
     ; return parameters:
     ;
     ; The value as computed above.
     ;
     ; useful notes and assumptions:
     ;
     ; Element        Description of use
     ; -------        ------------------
     ; FORTRAN        Intrinsic function RAN
     ;
     ; Update the seed and compute the value.
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
     ;                explicit support for real arithmetic
     ;
     ;/doc/ ************************************************************************
   */

#ifdef FPROTOTYPE
   float fifran(LONG* seed)
#else
      float fifran(seed)
      LONG* seed;                       /* Argument pointing to the seed */
#endif
   {
      double val;
      LONG seed1;

#ifdef ISUNALIGNED
      seed1 = get32s((char*)(seed));
      seed1 =  seed1 * 69069L + 1;
      put32s((char*)(seed),seed1);
      seed1 = seed1 >> 8;
#else
      *seed =  *seed * 69069L + 1;
      seed1 = *seed >> 8;
#endif
      seed1 &= 0xffffff;
      val = (double)(seed1)/16777216.0;
      return (float) val;
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; name of routine: FIFSIN -- FORTRAN Intrinsic Function SIN
     ;
     ; Computes the sine of a floating point argument using single precision.
     ;
     ; calling parameters:
     ;
     ; return parameters:
     ;
     ; The value as computed above.
     ;
     ; useful notes and assumptions:
     ;
     ; Element        Description of use
     ; -------        ------------------
     ; FORTRAN        Intrinsic function SIN
     ;
     ; Simply use the sine-cosine function to compute the result.
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
     ;                explicit support for real arithmetic
     ;
     ;/doc/ ************************************************************************
   */

#ifdef FPROTOTYPE
   float fifsin(float x)
#else
      float fifsin(x)
      float x;                          /* Argument containing the value */
#endif
   {
      return fifsncs(x,0);
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; name of routine: FIFSIND -- FORTRAN Intrinsic Function SIND
     ;
     ; Computes the sine of a floating point argument in degrees using single
     ; precision.
     ;
     ; calling parameters:
     ;
     ; return parameters:
     ;
     ; The value as computed above.
     ;
     ; useful notes and assumptions:
     ;
     ; Element        Description of use
     ; -------        ------------------
     ; FORTRAN        Intrinsic function SIND with a float argument
     ;
     ; Compute the value using the standard sin function.
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
     ;                explicit support for real arithmetic
     ;
     ;/doc/ ************************************************************************
   */

#ifdef FPROTOTYPE
   float fifsind(float x)
#else
      float fifsind(x)
      float x;                          /* Argument containing the value */
#endif
   {
      float ans;

      ans = (float)sin((double) x * 3.141592654 / 180.0);
      return ans;
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; name of routine: FIFSQRT -- FORTRAN Intrinsic Function SQRT
     ;
     ; Computes the square root of a float value.
     ;
     ; calling parameters:
     ;
     ; return parameters:
     ;
     ; The float result.
     ;
     ; useful notes and assumptions:
     ;
     ; Element        Description of use
     ; -------        ------------------
     ; FORTRAN        The sqrt function operator when applied to a float
     ;
     ; Simply use the standard sqrt function.
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
     ;                explicit support for real arithmetic
     ;
     ;/doc/ ************************************************************************
   */

#ifdef FPROTOTYPE
   float fifsqrt(float x)
#else
      float fifsqrt(x)
      float x;                          /* Value to be squared */
#endif
   {
      return (float)(sqrt((double)(x)));
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; name of routine: FIFTAN -- FORTRAN Intrinsic Function TAN
     ;
     ; Computes the tangent of a floating point argument.
     ;
     ; calling parameters:
     ;
     ; return parameters:
     ;
     ; The value as computed above.
     ;
     ; useful notes and assumptions:
     ;
     ; Element        Description of use
     ; -------        ------------------
     ; FORTRAN        Intrinsic function TAN
     ;
     ; Force the value back to single precision and then compute the tangent. The
     ; problem is that the tangent approaches infinity and is, therefore, very
     ; sensitive to the extra precision introduced via the promotion to double
     ; precision.
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
     ;                explicit support for real arithmetic
     ;
     ;/doc/ ************************************************************************
   */

#ifdef FPROTOTYPE
   float fiftan(float x)
#else
      float fiftan(x)
      float x;                          /* Argument containing the value */
#endif
   {
#ifdef ANSPLAT
#define zero  0.0F
#else
#define zero  0.0
#endif
      double temp;
      int n;
      double y;
      double f;
      double g;
      double xnum;
      double xden;
#ifdef LONGDBL
      long double xn;
#else
      double xn;
#endif
  
      temp = x;
      y = fabs(temp);
      if(y > 1.0e8) return zero;
      if(modf(x*0.63661977,&xn) >= 0.5) xn += 1.0;
      n = (int) xn;
      f = x-xn*1.5708008;
      f += xn*4.4544551e-6;
      if(fabs(f) < 1.0e-8) 
      {
         xnum = f;
         xden = 1.0;
      } 
      else 
      {
         g = f*f;
         xnum = ((-0.17861707e-4*g+0.34248878e-2)*g
                 -0.1333835)*g*f+f;
         xden = (((0.49819434e-6*g-0.31181532e-3)*g
                  +0.25663832e-1)*g-0.46671683)*g+0.1e1;
      }
      if(n & 1) xnum = -xnum;
      if(!(n & 1)) return (float) (xnum/xden);
      return (float) (xden/xnum);
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; name of routine: FIFTAND -- FORTRAN Intrinsic Function TAND
     ;
     ; Computes the tangent of a floating point argument in degrees using single
     ; precision.
     ;
     ; calling parameters:
     ;
     ; return parameters:
     ;
     ; The value as computed above.
     ;
     ; useful notes and assumptions:
     ;
     ; Element        Description of use
     ; -------        ------------------
     ; FORTRAN        Intrinsic function TAND with a float argument
     ;
     ; Compute the value using the standard tangent function.
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
     ;                explicit support for real arithmetic
     ;
     ;/doc/ ************************************************************************
   */

#ifdef FPROTOTYPE
   float fiftand(float x)
#else
      float fiftand(x)
      float x;                          /* Argument containing the value */
#endif
   {
      float ans;

      ans = (float) tan((double) x * 3.141592654 / 180.0);
      return ans;
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; name of routine: fifround -- FORTRAN Intrinsic Function Round
     ;
     ; Rounds the nearest integer to its double precision argument. In particular:
     ;
     ;      fifround(a) = fifdint(a+0.5001) if a >= 0.0
     ;                  = fifdint(a-0.5001) if a <= 0.0
     ;
     ; calling parameters:
     ;
     ; return parameters:
     ;
     ; The value as computed above.
     ;
     ; useful notes and assumptions:
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; creation date: 06/14/1988
     ;
     ;/doc/ ************************************************************************
   */

#ifdef FPROTOTYPE
   double fifround(double a)
#else
      double fifround(a)
      double a;                         /* Argument containing the value */
#endif
   {
      int sign;

      if(a < 0.0) {
         a = -a;
         sign = 1;
      }
      else sign = 0;
      a = floor(a + 0.5001);
      if(sign) a = -a;
      return a;
   }

   // fifsyst

   /*
     ;/hdr/ ************************************************************************
     ;
     ; Copyright 1986-1998 Promula Development Corporation ALL RIGHTS RESERVED
     ;
     ; name of module: FifSyst.c -- Fortran System Interface
     ;
     ; global symbols defined:
     ;
     ; useful notes and assumptions:
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;         author: (LCC) Lois Carver, Promula Development Corporation
     ;
     ; edit 08/04/98 LCC: Converted headers and comments to new standard.
     ;
     ; edit 06/17/98 FKG: Changed names of global symbols to conform to the 
     ;                    Promula version 7.08 API naming conventions. 
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
     ;                an interface to the operationg system.
     ;
     ;/hdr/ ************************************************************************
   */
#define OSYSFUNC                  /* Gives access to operating sys functions */
#define PRIOFUNC                  /* Gives access to PROMULA I/O functions */
#define RAWMFUNC                  /* Gives access to raw memory functions */
#define STRGFUNC                  /* Gives access to the string functions */
   // #include "platform.h"             /* Define the platform hosting this code */
   // #include "pfclib.h"               /* Define PROMULA.FORTRAN library symbols */
   // #include "chrapi.h"               /* Define character manipulation interface */

   extern const char** comlin;             /* Pointer to command line arguments */
   extern int    fioconv;            /* General dialect convention flags */
   extern int    ncomlin;            /* Number of command line arguments */
   extern txtfile fioscon;

   /*
     ;/doc/ ***********************************************************************
     ;
     ; name of routine: FIFEXIT -- FORTRAN Exit Subroutine
     ;
     ; Exits to the operating system.
     ;
     ; calling parameters:
     ;
     ; return parameters:
     ;
     ; None, the function exits to the operating system and never returns.
     ;
     ; useful notes and assumptions:
     ;
     ; Element        Description of use
     ; -------        ------------------
     ; FORTRAN        External subroutine EXIT
     ;
     ; Simply exit to the operating system.
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
     ;                an interface to the operationg system.
     ;
     ;/doc/ ************************************************************************
   */

#ifdef FPROTOTYPE
   void fifexit(LONG* numb)
#else
      void fifexit(numb)
      LONG* numb;                    /* Message to be displayed at console */
#endif
   {
      int iv;
      int i;
      unsigned char rec[2];
      binfile bin;

      if(numb == NULL) iv = 0;
      else {
#ifdef ISUNALIGNED
         iv = (int) get32s((char*)(numb));
#else
         iv = (int) *numb;
#endif
         if(comlin != NULL) {
            for(i = 1; i < ncomlin; i++) {
               if(*comlin[i] == 'F') break;
            }
            if(i < ncomlin) {
               bin = opnbinf(comlin[i]+1);
               if(binopner(bin) == 0) {
                  rec[0] = (unsigned char)(iv);
                  wrbinf(bin,rec,1);
                  clsbinf(bin);
               }
            }
         }  
      }
      exit(iv);
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; name of routine: FIFGETARG -- FORTRAN Get Command Line Arguments
     ;
     ; Copies the kth command line argument into a string variable arg. The 0th
     ; argument is the command name.
     ;
     ; calling parameters:
     ;
     ; return parameters:
     ;
     ; None, the function is void.
     ;
     ; useful notes and assumptions:
     ;
     ; Element        Description of use
     ; -------        ------------------
     ; FORTRAN        External subroutine GETARG
     ;
     ; Simply copy the command line arguments into the user supplied character
     ; string.
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
     ;                an interface to the operationg system.
     ;
     ;/doc/ ************************************************************************
   */

#ifdef FPROTOTYPE
   void fifgetarg(LONG k,char* cl,int ncl)
#else
      void fifgetarg(k, cl,ncl)
      LONG k;
   char* cl;
   int ncl;
#endif
   {
      int ic;

      filmem(cl,ncl,' ');
      if (k < ncomlin) 
      {
         ic = strlen(comlin[k]);
         if(ic > ncl) ic = ncl;
         if(ic) cpymem(comlin[k],cl,ic);
      }
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; name of routine: FIFGETCL -- FORTRAN Get Command Line Subroutine
     ;
     ; Copies the command line parameters as entered by the user into a string
     ; variable. The individual parameters are separated by spaces.
     ;
     ; calling parameters:
     ;
     ; return parameters:
     ;
     ; None, the function is void.
     ;
     ; useful notes and assumptions:
     ;
     ; Element        Description of use
     ; -------        ------------------
     ; FORTRAN        External subroutine GETCL
     ;
     ; Simply copy the command line arguments into the user supplied character
     ; string.
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
     ;                an interface to the operationg system.
     ;
     ;/doc/ ************************************************************************
   */

#ifdef FPROTOTYPE
   void fifgetcl(char* cl,int ncl)
#else
      void fifgetcl(cl,ncl)
      char* cl;
   int ncl;
#endif
   {
      int ic;
      int ll;
      int il;
      int nc;

      nc = ncl;
      filmem(cl,nc,' ');
      ll = 0;
      for(il = 1; il < ncomlin; il++) 
      {
         ic = strlen(comlin[il]);
         if(ic > ncl) ic = ncl;
         if(ic) cpymem(comlin[il],cl+ll,ic);
         ll += ic;
         ncl -= ic;
         if(ncl) 
         {
            cl[ll++] = ' ';
            ncl--;
         }
      }
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; name of routine: FIFGETENV -- FORTRAN Get Value of Environment Variables
     ;
     ; Searches the environment list for a string of the form ename-value and
     ; returns value in evalue if such string is present, otherwise fills evalue
     ; with blanks.
     ;
     ; calling parameters:
     ;
     ; return parameters:
     ;
     ; None, the function is void.
     ;
     ; useful notes and assumptions:
     ;
     ; Element        Description of use
     ; -------        ------------------
     ; FORTRAN        External subroutine GETENV
     ;
     ; Use the system "getenv" function to obtain the variable value in C form
     ; and then store it in the return string using FORTRAN conventions.
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
     ;                an interface to the operationg system.
     ;
     ;/doc/ ************************************************************************
   */

#ifdef FPROTOTYPE
   void fifgetenv(char* ename,int lenin,char* evalue,int lenout)
#else
      void fifgetenv(ename,lenin,evalue,lenout)
      char* ename;                   /* Name of environment variable */
   int lenin;                     /* Length of name */
   char* evalue;                  /* Returns value of environment variable */
   int lenout;                    /* Length available for variable value */
#endif
   {
      int i;                    /* Dummy counter */
      char str[100];            /* Temporary storage for var name, C form */
      char* s1;                 /* Pointer to variable value */
      int len;                  /* Length of variable value */

      if(lenin > 98) lenin = 98;
      for(i = 0; i < lenin; i++) 
      {
         str[i] = *ename++;
         if(str[i] == ' ' || str[i] == '\t') break;
      }
      str[i] = '\0';
      s1 = (char*) getenv(str);
      if(s1 == NULL) len = 0;
      else 
      {
         len = strlen(s1);
         if(len > lenout) len = lenout;
         if(len > 0) cpymem(s1,evalue,len);
      }
      if(len < lenout) filmem(evalue+len,lenout-len,' ');
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; name of routine: FIFIARGC -- FORTRAN Get Command Line Argument Count
     ;
     ; Returns the index number of command line arguments -- i.e. the number of
     ; arguments minus one, where the program name counts as one argument.
     ;
     ; calling parameters:
     ;
     ; return parameters:
     ;
     ; The number of command line arguments minus one.
     ;
     ; useful notes and assumptions:
     ;
     ; Element        Description of use
     ; -------        ------------------
     ; FORTRAN        External function IARGC
     ;
     ; Simply return the value. Note that this value is saved by the "ftnini"
     ; function.
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
     ;                an interface to the operationg system.
     ;
     ;/doc/ ************************************************************************
   */

#ifdef FPROTOTYPE
   LONG fifiargc(void)
#else
      LONG fifiargc()
#endif
      {
         return (LONG)(ncomlin-1);
      }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; name of routine: FTNPAUSE -- FORTRAN Pause Statement
     ;
     ; Executes the "pause" operation of the FORTRAN pause statement. This
     ; function assumes that some message has already been written to the console.
     ; In particular, it writes the message
     ;
     ;     PAUSE message
     ;
     ; to console and then waits for any new record from "stdin". If "stdin" and
     ; "stdout" are redirected, then this function may not produce the desired
     ; result.
     ;
     ; calling parameters:
     ;
     ; return parameters:
     ;
     ; None, this function is void.
     ;
     ; useful notes and assumptions:
     ;
     ; Simply display the message and await a response.
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
     ;                an interface to the operationg system.
     ;
     ;/doc/ ************************************************************************
   */

#ifdef FPROTOTYPE
   void ftnpause(const char* message,int nmes)
#else
      void ftnpause(message,nmes)
      char* message;                 /* Message */
   int nmes;
#endif
   {
      char answer[20];

      if(!(fioconv & ZDPAUSE)) wrtxtf(fioscon,"PAUSE ",6);
      if(fioconv & ZDFLEXF)
      {
         wrteol(fioscon);
         if(message == NULL) wrtxtf(fioscon,"*** PAUSE ***",13);
         else
         {
            wrtxtf(fioscon,"*** PAUSE ***  Message is:",26);
            wrteol(fioscon);
            if(nmes == 0) nmes = strlen(message);
            if(nmes != 0) wrtxtf(fioscon,message,nmes);
         }
         wrteol(fioscon);
         wrtxtf(fioscon,"Type NEW-LINE to continue.",26);
      }   
      else if(message != NULL)
      {
         if(nmes == 0) nmes = strlen(message);
         if(nmes != 0) wrtxtf(fioscon,message,nmes);
      }
      wrteol(fioscon);
      if(fioconv & ZDPAUSE) 
      {
         rdtxtf(SINPUT,answer,18);
      }
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; name of routine: FTNSTOP -- FORTRAN Stop Statement
     ;
     ; Executes the FORTRAN stop statement by sending a message to the console,
     ; stderr, not the screen, stdout, and making a normal exit. It should be
     ; pointed out that this is one of those conventions that reasonable people
     ; do disagree about. Changing the output file to stdout is easily
     ; accomplished in the source for this function via a defined variable
     ; "console".
     ;
     ; calling parameters:
     ;
     ; return parameters:
     ;
     ; None, this function does not return to the calling function.
     ;
     ; useful notes and assumptions:
     ;
     ; Simply display the message and exit.
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
     ;                an interface to the operationg system.
     ;
     ;/doc/ ************************************************************************
   */

#ifdef FPROTOTYPE
   void ftnstop(const char* message)
#else
      void ftnstop(message)
      const char* message;                 /* Message to be displayed at console */
#endif
   {
      LONG iexit;

      iexit = 0;
      if(message != NULL)
      {
         wrtxtf(fioscon,message,strlen(message));
         wrteol(fioscon);
         iexit = ChrShortFromString(&message,0);
      }
      fifexit(&iexit);
   }

   // fifint4
   /*
     ;/hdr/ ************************************************************************
     ;
     ; FifInt4.c: Fortran Integer*4 Functions
     ;
     ;/hdr/ ************************************************************************
   */
#define MATHFUNC                  /* Gives access to mathematical functions */
   // #include "platform.h"             /* Define the platform hosting this code */
   // #include "pfclib.h"               /* Define PROMULA.FORTRAN library symbols */

   /*
     ;/doc/ ***********************************************************************
     ;
     ; fifeqf: FORTRAN Intrinsic Function EQF
     ;
     ; Synopsis of Service:
     ;
     ; #include "fortran.h"                 pFortran runtime library interface
     ;
     ; int fifeqf                           FORTRAN intrinsic function EQF
     ; (
     ;    float a                           First value to be compared
     ;    float b                           Second value to be compared
     ; )
     ;
     ; Description of Service:
     ;
     ; This service compares two floating point numbers to determine if they would
     ; be equal in single precision floating point form. The implementation does not
     ; do a floating point comparison because float values are truncated while
     ; explicit constants are not. Instead, it does a fixed-point comparison of the
     ; float representation instead.
     ;
     ; Properties of Service:
     ;
     ; a            First value to be compared
     ;
     ; b            Second value to be compared
     ;
     ; Return Value from Service:
     ;
     ; The integer result of the comparison -- zero if false, one if true. 
     ;
     ;/doc/ ************************************************************************
   */
#ifdef FPROTOTYPE
   int fifeqf(float a,float b)
#else
      int fifeqf(a,b)
      float a;                       /* First value to be compared */
   float b;                       /* Second value to be compared */
#endif
   {
      float af;
      float bf;

      af = (float)(a);
      bf = (float)(b);
      //      return *(LONG*)&af == *(LONG*)&bf;
      return af == bf;
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; fifi4log: FORTRAN Intrinsic Function I4LOG
     ;
     ; Synopsis of Service:
     ;
     ; #include "fortran.h"                 pFortran runtime library interface
     ;
     ; LONG fifi4log                        FORTRAN intrinsic function I4LOG
     ; (
     ;    LONG a                            Logical*4 to be converted
     ; )
     ;
     ; Description of Service:
     ;
     ; This service converts a logical*4 value to an integer*4 by extending the bit
     ; in position 0 to all of the bits in the value. The approach simply reads the
     ; bit in position 0 and returns a -1 or a zero depending upon whether the bit
     ; is 1 or 0.
     ;
     ; Properties of Service:
     ;
     ; a            Logical*4 to be converted
     ;
     ; Return Value from Service:
     ;
     ; The long integer result.
     ;
     ;/doc/ ************************************************************************
   */
#ifdef FPROTOTYPE
   LONG fifi4log(LONG a)
#else
      LONG fifi4log(a)
      LONG a;                        /* Logical*4 to be converted */
#endif
   {
      if(a & 1) a = -1;
      else a = 0;
      return a;
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; fifiabS: FORTRAN Intrinsic Function IABS
     ;
     ; Synopsis of Service:
     ;
     ; #include "fortran.h"                 pFortran runtime library interface
     ;
     ; LONG fifiabs                         FORTRAN intrinsic function IABS
     ; (
     ;    LONG a                            Value whose absolute value is needed
     ; )
     ;
     ; Description of Service:
     ;
     ; This service implements the intrinsic function IABS and generic function ABS
     ; when used with a long integer argument. It computes the absolute value of a
     ; long integer argument -- i.e. if the value is negative, make it positive.
     ;
     ; Properties of Service:
     ;
     ; a            Value whose absolute value is needed
     ;
     ; Return Value from Service:
     ;
     ; The long integer result.
     ;
     ;/doc/ ************************************************************************
   */
#ifdef FPROTOTYPE
   LONG fifiabs(LONG a)
#else
      LONG fifiabs(a)
      LONG a;                        /* Value whose absolute value is needed */
#endif
   {
      if(a < 0) a = -a;
      return a;
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; fifichar: FORTRAN Intrinsic Function ICHAR
     ;
     ; Synopsis of Service:
     ;
     ; #include "fortran.h"                 pFortran runtime library interface
     ;
     ; Description of Service:
     ; Properties of Service:
     ; Return Value from Service:
     ;
     ; Converts a character code into its numeric display code, or "lexical value"
     ; or "collating weight". The point of this function is that character values
     ; on the host processor are not necessarily the same as those on the machine
     ; for which a given FORTRAN program was written. All character "value" 
     ; references in a source FORTRAN program are passed through this function
     ; either by the translator directly or by the other runtime functions included
     ; in this library. Note that if you wish this function to return some value
     ; other than the host processor values then you must modify it. Typically
     ; this modification would take the form of a lookup table reference.
     ;
     ; calling parameters:
     ;
     ; return parameters:
     ;
     ; The character value directly.
     ;
     ; useful notes and assumptions:
     ;
     ; Element        Description of use
     ; -------        ------------------
     ; FORTRAN        Intrinsic function ICHAR
     ; ftncms         Compare two characters in terms of their lexical value.
     ;
     ; Just return the integer value.
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
     ;                explicit INTEGER*4 support.
     ;
     ;/doc/ ************************************************************************
   */

#ifdef FPROTOTYPE
   int fifichar(char* c1)
#else
      int fifichar(c1)
      char* c1;                      /* Character to be converted */
#endif
   {
      return (int) *(unsigned char*) c1;
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; fifidim: FORTRAN Intrinsic Function IDIM
     ;
     ; Synopsis of Service:
     ;
     ; #include "fortran.h"                 pFortran runtime library interface
     ;
     ; Description of Service:
     ; Properties of Service:
     ; Return Value from Service:
     ;
     ; Computes a positive difference.
     ;
     ; calling parameters:
     ;
     ; return parameters:
     ;
     ; If a1 is greater than a2 then the value of a1 - a2 is returned, else zero
     ; is returned.
     ;
     ; useful notes and assumptions:
     ;
     ; Element        Description of use
     ; -------        ------------------
     ; FORTRAN        Intrinsic function IDIM and generic function DIM when used
     ;                with long integer arguments.
     ;
     ; Do the comparison of a1 and a2 and return the value specified above.
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
     ;                explicit INTEGER*4 support.
     ;
     ;/doc/ ************************************************************************
   */

#ifdef FPROTOTYPE
   LONG fifidim(LONG a1,LONG a2)
#else
      LONG fifidim(a1,a2)
      LONG a1;                       /* First value in difference */
   LONG a2;                       /* Second value in difference */
#endif
   {
      if(a1 > a2) return a1 - a2;
      else return 0L;
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; fifidint: FORTRAN Intrinsic Function IDINT
     ;
     ; Synopsis of Service:
     ;
     ; #include "fortran.h"                 pFortran runtime library interface
     ;
     ; Description of Service:
     ; Properties of Service:
     ; Return Value from Service:
     ;
     ; Truncates its double precision argument to a long integer value. In
     ; particular:
     ;     fifidint(a) = 0 if |a| < 1
     ;               = the largest integer with the same sign as a that
     ;                 does not exceed a if |a| >= 1
     ;
     ; calling parameters:
     ;
     ; return parameters:
     ;
     ; The value as computed above.
     ;
     ; useful notes and assumptions:
     ;
     ; Element        Description of use
     ; -------        ------------------
     ; FORTRAN        Intrinsic function IDINT and the generic functions IFIX,
     ;                INT, INT2, and INT4
     ; fifi2nint      Compute nearest short integer
     ; fifnint        Compute nearest long integer
     ;
     ; This value is simply the long cast of the integal return value from the C
     ; "modf" function.
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
     ;                explicit INTEGER*4 support.
     ;
     ;/doc/ ************************************************************************
   */

#ifdef FPROTOTYPE
   LONG fifidint(double a)
#else
      LONG fifidint(a)
      double a;                      /* Argument containing the value */
#endif
   {
#ifdef LONGDBL
      long double temp;
      modf(a,&temp);
      a = temp;
#else
      modf(a,&a);
#endif
      return (LONG) a;
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; FIFINDEX: FORTRAN Intrinsic Function INDEX
     ;
     ; Synopsis of Service:
     ;
     ; #include "fortran.h"                 pFortran runtime library interface
     ;
     ; Description of Service:
     ; Properties of Service:
     ; Return Value from Service:
     ;
     ; Returns the location of a substring within a string. Note that the
     ; arguments are two FORTRAN style strings, which means that the length of
     ; each string must also be sent.
     ;
     ; calling parameters:
     ;
     ; return parameters:
     ;
     ; If the substring occurs within the main string, the result is an integer
     ; indicating the starting position (relative to 1) of the first occurrence
     ; of the substring within the main string. If the substring does not occur
     ; with the main string, then a zero is returned.
     ;
     ; useful notes and assumptions:
     ;
     ; Element        Description of use
     ; -------        ------------------
     ; FORTRAN        Intrinsic function INDEX
     ;
     ; Compute the number of positions at the front of the string in which the
     ; substring can begin. Then search for the first character of the substring,
     ; followed by its remaining characters.
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
     ;                explicit INTEGER*4 support.
     ;
     ;/doc/ ************************************************************************
   */

#ifdef FPROTOTYPE
   int fifindex(CONST char* s,int ns,CONST char* c,int nc)
#else
      int fifindex(s,ns,c,nc)
      char* s;                       /* Main string */
   int ns;                        /* Length of main string */
   char* c;                       /* Substring */
   int nc;                        /* Length of substring */
#endif
   {
      char* a;                  /* Pointer to substring start main string */
      int i;                    /* Dummy pointer moving through main strng */
      int j;                    /* Dummy pointer moving through substring */
      int nf;                   /* Num possible first characters in main */
      nf = ns - nc + 1;
      for(i = 0; i < nf; i++) 
      {
         if(*c == s[i]) 
         {
            for(j = 1, a = (char*)(s+i); j < nc; j++) if(c[j] != *(a+j)) break;
            if(j == nc) 
            {
               return i+1;
            }
         }
      }
      return 0;
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; fifipow: FORTRAN Intrinsic Function IPOW
     ;
     ; Synopsis of Service:
     ;
     ; #include "fortran.h"                 pFortran runtime library interface
     ;
     ; Description of Service:
     ; Properties of Service:
     ; Return Value from Service:
     ;
     ; Raises a long integer value to a long integer power.
     ;
     ; calling parameters:
     ;
     ; return parameters:
     ;
     ; The long integer result.
     ;
     ; useful notes and assumptions:
     ;
     ; Element        Description of use
     ; -------        ------------------
     ; FORTRAN        The ** operator when applied to long integers
     ;
     ; Simply use the standard C function "pow" to compute the result.
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
     ;                explicit INTEGER*4 support.
     ;
     ;/doc/ ************************************************************************
   */

#ifdef FPROTOTYPE
   LONG fifipow(LONG a,LONG b)
#else
      LONG fifipow(a,b)
      LONG a;                        /* Value to be raised to a power */
   LONG b;                        /* Power to be used */
#endif
   {
      int expon;
      int i;
      int p2;
      int ht;
      LONG temp;
      LONG pw[6];

      if(b <= 0) 
      {
         if(b == 0 || a == 1) return 1L;
         else return 0L;
      }
      temp = a;
      ht = 0;
      expon = (int) b;
      for(i = 0, p2 = 32; i < 6 && expon > 0; i++, p2 /= 2) 
      {
         while(expon > p2) 
         {
            if(!ht) 
            {
               pw[5] = a;
               for(ht = 5; ht > i; ht--) pw[ht-1] = pw[ht] * pw[ht];
               ht = 1;
            }
            temp *= pw[i];
            expon -= p2;
         }
      }
      return temp;
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; fifisign: FORTRAN Intrinsic Function ISIGN
     ;
     ; Synopsis of Service:
     ;
     ; #include "fortran.h"                 pFortran runtime library interface
     ;
     ; Description of Service:
     ; Properties of Service:
     ; Return Value from Service:
     ;
     ; Returns a value after the transfer of a sign. The result is |mag| if sgn
     ; is at least zero, else it is -|mag|.
     ;
     ; calling parameters:
     ;
     ; return parameters:
     ;
     ; The value as computed above.
     ;
     ; useful notes and assumptions:
     ;
     ; Element        Description of use
     ; -------        ------------------
     ; FORTRAN        Intrinsic function ISIGN and generic function SIGN when
     ;                used with long integer arguments.
     ;
     ; If the magnitude and the sign do not have the same sign, return the
     ; negative of the magnitude; else simply return the magnitude.
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
     ;                explicit INTEGER*4 support.
     ;
     ;/doc/ ************************************************************************
   */

#ifdef FPROTOTYPE
   LONG fifisign(LONG mag,LONG sgn)
#else
      LONG fifisign(mag,sgn)
      LONG mag;                      /* The magnitude for the result */
   LONG sgn;                      /* The sign for the result */
#endif
   {
      if((mag > 0 && sgn < 0) || (mag < 0 && sgn >= 0)) return -mag;
      else return mag;
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; fifmax0: FORTRAN Intrinsic Function MAX0
     ;
     ; Synopsis of Service:
     ;
     ; #include "fortran.h"                 pFortran runtime library interface
     ;
     ; Description of Service:
     ; Properties of Service:
     ; Return Value from Service:
     ;
     ; Determines which of two long values is the largest.
     ;
     ; calling parameters:
     ;
     ; return parameters:
     ;
     ; The largest long value.
     ;
     ; useful notes and assumptions:
     ;
     ; Element        Description of use
     ; -------        ------------------
     ; FORTRAN        Intrinsic function MAX0
     ;
     ; Compare the two values and return the largest.
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
     ;                explicit INTEGER*4 support.
     ;
     ;/doc/ ************************************************************************
   */

#ifdef FPROTOTYPE
   LONG fifmax0(LONG a1,LONG a2)
#else
      LONG fifmax0(a1,a2)
      LONG a1;                       /* First value to be compared */
   LONG a2;                       /* Second value to be compared */
#endif
   {
      if(a1 >= a2) return a1;
      else return a2;
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; fifmax1: FORTRAN Intrinsic Function MAX1
     ;
     ; Synopsis of Service:
     ;
     ; #include "fortran.h"                 pFortran runtime library interface
     ;
     ; Description of Service:
     ; Properties of Service:
     ; Return Value from Service:
     ;
     ; Determines which of two floating point values is the largest.
     ;
     ; calling parameters:
     ;
     ; return parameters:
     ;
     ; The largest value converted to a long integer.
     ;
     ; useful notes and assumptions:
     ;
     ; Element        Description of use
     ; -------        ------------------
     ; FORTRAN        Intrinsic functions MAX1
     ; 
     ; Compare the two values and return the largest.
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
     ;                explicit INTEGER*4 support.
     ;
     ;/doc/ ************************************************************************
   */

#ifdef FPROTOTYPE
   LONG fifmax1(float a1,float a2)
#else
      LONG fifmax1(a1,a2)
      float a1;                      /* First value to be compared */
   float a2;                      /* Second value to be compared */
#endif
   {
      if(a1 >= a2) return (LONG) a1;
      else return (LONG) a2;
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; fifmin0: FORTRAN Intrinsic Function MIN0
     ;
     ; Synopsis of Service:
     ;
     ; #include "fortran.h"                 pFortran runtime library interface
     ;
     ; Description of Service:
     ; Properties of Service:
     ; Return Value from Service:
     ;
     ; Determines which of two long values is the smallest.
     ;
     ; calling parameters:
     ;
     ; return parameters:
     ;
     ; The smallest long value.
     ;
     ; useful notes and assumptions:
     ;
     ; Element        Description of use
     ; -------        ------------------
     ; FORTRAN        Intrinsic function MIN0
     ;
     ; Compare the two values and return the smallest.
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
     ;                explicit INTEGER*4 support.
     ;
     ;/doc/ ************************************************************************
   */

#ifdef FPROTOTYPE
   LONG fifmin0(LONG a1,LONG a2)
#else
      LONG fifmin0(a1,a2)
      LONG a1;                       /* First value to be compared */
   LONG a2;                       /* Second value to be compared */
#endif
   {
      if(a1 <= a2) return a1;
      else return a2;
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; fifmin1: FORTRAN Intrinsic Function MIN1
     ;
     ; Synopsis of Service:
     ;
     ; #include "fortran.h"                 pFortran runtime library interface
     ;
     ; Description of Service:
     ; Properties of Service:
     ; Return Value from Service:
     ;
     ; Determines which of two floating point values is the smallest.
     ;
     ; calling parameters:
     ;
     ; return parameters:
     ;
     ; The smallest value converted to a long integer.
     ;
     ; useful notes and assumptions:
     ;
     ; Element        Description of use
     ; -------        ------------------
     ; FORTRAN        Intrinsic functions MIN1
     ; 
     ; Compare the two values and return the smallest.
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
     ;                explicit INTEGER*4 support.
     ;
     ;/doc/ ************************************************************************
   */

#ifdef FPROTOTYPE
   LONG fifmin1(float a1,float a2)
#else
      LONG fifmin1(a1,a2)
      float a1;                      /* First value to be compared */
   float a2;                      /* Second value to be compared */
#endif
   {
      if(a1 <= a2) return (LONG) a1;
      else return (LONG) a2;
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; fifmod: FORTRAN Intrinsic Function MOD
     ;
     ; Synopsis of Service:
     ;
     ; #include "fortran.h"                 pFortran runtime library interface
     ;
     ; Description of Service:
     ; Properties of Service:
     ; Return Value from Service:
     ;
     ; Computes the value of the remainder of num devided by dem. If dem is zero,
     ; then the result is zero.
     ;
     ; calling parameters:
     ;
     ; return parameters:
     ;
     ; The value as computed above.
     ;
     ; useful notes and assumptions:
     ;
     ; Element        Description of use
     ; -------        ------------------
     ; FORTRAN        Generic function MOD when used with long integer arguments.
     ;
     ; This value is the result of num % dem if dem is nonzero, else it is zero.
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
     ;                explicit INTEGER*4 support.
     ;
     ;/doc/ ************************************************************************
   */

#ifdef FPROTOTYPE
   LONG fifmod(LONG num,LONG dem)
#else
      LONG fifmod(num,dem)
      LONG num;                      /* The numerator for the calculation */
   LONG dem;                      /* The denominator for the calculation */
#endif
   {
      if(!dem) return 0L;
      else return num % dem;
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; fifnef: FORTRAN Intrinsic Function NEF
     ;
     ; Synopsis of Service:
     ;
     ; #include "fortran.h"                 pFortran runtime library interface
     ;
     ; Description of Service:
     ; Properties of Service:
     ; Return Value from Service:
     ;
     ; Compares two floating point numbers to determine if they would be unequal
     ; in single precision floating point form.
     ;
     ; calling parameters:
     ;
     ; return parameters:
     ;
     ; The short integer result of the comparison.
     ;
     ; useful notes and assumptions:
     ;
     ; Element        Description of use
     ; -------        ------------------
     ; FORTRAN        When two float values are compared for equality
     ;
     ; We cannot do a floating point comparison because float values are truncated
     ; while explicit constants are not. We do a fixed-point comparison of the
     ; float representation instead.
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
     ;                explicit INTEGER*4 support.
     ;
     ;/doc/ ************************************************************************
   */

#ifdef FPROTOTYPE
   int fifnef(float a,float b)
#else
      int fifnef(a,b)
      float a;                       /* First value to be compared */
   float b;                       /* Second value to be compared */
#endif
   {
      float af;
      float bf;

      af = (float)(a);
      bf = (float)(b);
      //      return *(LONG*)&af != *(LONG*)&bf;
      return af != bf;
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; fifnint: FORTRAN Intrinsic Function NINT
     ;
     ; Synopsis of Service:
     ;
     ; #include "fortran.h"                 pFortran runtime library interface
     ;
     ; Description of Service:
     ; Properties of Service:
     ; Return Value from Service:
     ;
     ; Computes the nearest integer to its double precision argument. In particular:
     ; 
     ;      fifnint(a) = fifidint(a+0.5) if a >= 0.0
     ;                 = fifidint(a-0.5) if a <= 0.0
     ;
     ; where "fifidint" is the FORTRAN intrinsic function IDINT.
     ;
     ; calling parameters:
     ;
     ; return parameters:
     ;
     ; The value as computed above.
     ;
     ; useful notes and assumptions:
     ;
     ; Element        Description of use
     ; -------        ------------------
     ; FORTRAN        Intrinsic functions IDNINT and NINT
     ;
     ; Simply do the calculation as specified above.
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
     ;                explicit INTEGER*4 support.
     ;
     ;/doc/ ************************************************************************
   */

#ifdef FPROTOTYPE
   LONG fifidint(double a);
#else
   extern LONG fifidint();         /* FORTRAN Intrinsic Function IDINT */
#endif

#ifdef FPROTOTYPE
   LONG fifnint(double a)
#else
      LONG fifnint(a)
      double a;                      /* Argument containing the value */
#endif
   {
      if(a < 0.0) a -= 0.5;
      else a += 0.5;
      return fifidint(a);
   }

   // fioread

   /*
     ;/hdr/ ************************************************************************
     ;
     ; Copyright 1986-1998 Promula Development Corporation ALL RIGHTS RESERVED
     ;
     ; name of module: FioRead.c -- Fortran I/O, Read Operations
     ;
     ; This module contains those functions which perform the operations needed
     ; particularly by the Fortran READ statement.
     ;
     ; global symbols defined:
     ;
     ; fiorbiv         Read FORTRAN Binary Values
     ; fiordb          Read FORTRAN Boolean Vector
     ; fiordc          Read FORTRAN Charactor Value Vector
     ; fiordd          Read FORTRAN Double Precision Vector
     ; fiordf          Read FORTRAN Floating Point Values
     ; fiordi          Read FORTRAN Short Integer Vector
     ; fiordl          Read FORTRAN Long Integer Vector
     ; fiords          Read FORTRAN String
     ; fiordt          Read FORTRAN Truth-Value Vector
     ; fiordu          Read FORTRAN Unsigned Char Vector
     ; fiordx          Read FORTRAN Complex Values
     ; fiordz          Read FORTRAN Double Complex Values
     ; fiorln          Read FORTRAN End-of-Line
     ; ftnread         FORTRAN Read Statement
     ;
     ; useful notes and assumptions:
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; edit 06/17/98 FKG: Changed names of global symbols to conform to the 
     ;                    Promula version 7.08 API naming conventions. 
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00
     ;
     ;/hdr/ ************************************************************************
   */
#define CHARTYPE                  /* Gives access to character type info */
#define MATHFUNC                  /* Gives access to mathematical functions */
#define RAWMFUNC                  /* Gives access to raw memory functions */
#define PRIOFUNC                  /* Gives access to PROMULA I/O functions */
#define STRGFUNC                  /* Access to string manipulation functions */
#define VARARGUS                  /* Gives access to variable arguments */
   // #include "platform.h"             /* Define the platform hosting this code */
   // #include "pfclib.h"               /* Define PROMULA.FORTRAN library symbols */
   // #include "chrapi.h"               /* Define character manipulation interface */

#ifdef FPROTOTYPE
   void   fifibit(unsigned char* bits,int ibit,int ival);
   void   fifrbit(unsigned char* bits,int nbyte);
   int    fifxbit(unsigned char* bits,int ibit);
   int    fioerror(int clear);
   void   fiofend(void);
   int    fiofini(char** fmt,int nfmt);
   int    fiointu(char* intu,int rsize,int action);
   int    fiolun(int lun,int action);
   int    fioname(char* strg,int ns,int status);
   void   fionxtf(void);
   void   fiorec(int irec);
   void   fiostatus(LONG* iostat,int error);
   double fiostod(char *str,int nstr);
   int    fiostio(int action);
   int    fiovfini(char** fmt,int nfmt);
#else
   extern void   fifibit();          /* Insert a bit */
   extern void   fifrbit();          /* Reverse bit byte order */
   extern int    fifxbit();          /* Extract a bit */
   extern int    fioerror();         /* Do requested error processing */
   extern void   fiofend();          /* End current format processing */
   extern int    fiofini();          /* Initialize A FORTRAN format */
   extern int    fiointu();          /* Establish FORTRAN internal unit */
   extern int    fiolun();           /* Establish FORTRAN unit number */
   extern int    fioname();          /* Establish FORTRAN unit by name */
   extern void   fionxtf();          /* Get next format specification */
   extern void   fiorec();           /* Position a FORTRAN file on a record */
   extern void   fiostatus();        /* Set FORTRAN I/O error status */
   extern double fiostod();          /* Convert string to double */
   extern int    fiostio();          /* Establish FORTRAN standard I/O */
   extern int    fiovfini();         /* Initialize a variable FORTRAN format */
#endif

   extern int      fioblkn;          /* Blanks are null input convention */
   extern int      fiobsex;          /* Is byte sex little-end first */
   extern int      fioconv;          /* General dialect convention flags */
   extern char     fiocrec[FIOBMAX]; /* Coded communications record */
   extern FIOFILE* fiocurf;          /* Pointer to current FORTRAN file */
   extern int      fioerc;           /* Error support information */
   extern int      fioerch;          /* Specifies whether error checking present */
   extern int      fioier;           /* Code for actual error encountered */
   extern int      fiofspec;         /* Current format specification */
   extern int      fioicol;          /* Number of current input character */
   extern int      fioiwd;           /* Display width */
   extern int      fiolcol;          /* Number of last output character */
   extern int      fiondec;          /* Number of implied decimal places */
   extern int      fionfmt;          /* Number of entries in format list */
   extern int      fiorpeat;         /* Current repeat count */
   extern char*    fiosadr;          /* Format string location */
   extern int      fioscal;          /* Floating point scale factor */
   extern LONG*    fiostat;          /* Returns an error code or zero */

   static int mode = 0;              /* 1 if waiting for MORE else 0 */
   static int iomode = 0;            /* 0 if binary read, else 1 */

   /*****************************************************************************/
   /*                                                                           */
   /* Copyright (C) 1988-98 PROMULA Development Corp., All Rights Reserved      */
   /*                                                                           */
   /* fiortxt: Read Next Text Record                                            */
   /*                                                                           */
   /* Author(s): Fred Goodman (FG)                                              */
   /*                                                                           */
   /* Creation date: 06/14/1988                                                 */
   /*                                                                           */
   /* FG 12/02/97 Update: The content of fiocurf is a char** pointer cast to    */
   /* a char* when the frsize is zero. This is the convention used to do the    */
   /* internal namelist reads for block data. The retreival of this pointer     */
   /* was made to reflect this fact exactly. This change was made necessary by  */
   /* the Data General platform which has a special representation for character*/
   /* pointers.                                                                 */
   /*                                                                           */
   /* Purpose: Reads the next physical text record into the coded communica-    */
   /* tions record for detailed processing. Both "internal" and "external"      */
   /* files are processed.                                                      */
   /*                                                                           */
   /* Argument(s): None                                                         */
   /*                                                                           */
   /* Return value: None, the function is void. The effect of its processing is */
   /* reflected in the various global format control variables.                 */
   /*                                                                           */
   /*****************************************************************************/

#ifdef FPROTOTYPE
   void fiortxt(void)
#else
      void fiortxt()
#endif
      {
         char** s1;
         char* intu;
         int ipos;
         int lpos;

         if(fiocurf->fstat1 & FREREAD) {
            fiocurf->fstat1 ^= FREREAD;
            fioicol = 0;
            return;
         }
         if(!(fiocurf->fstat & FINTERNAL)) {
            if(fiocurf->fstat & FDIRECT) {
               fiolcol = (int)(fiocurf->frsize);
               if(fioconv & ZDVAXFT) {
                  lpos = fiolcol+1;
                  if((int)rdbinf(*(binfile*)&(fiocurf->ffile),fiocrec,lpos) < lpos){
                     fioier = ERTX_EOF;
                     fioerror(0);
                  }
                  else {
                     fiocurf->fstat1 |= FREADLAST;
                     fiocrec[fiolcol] = '\0';
                  }
                  goto fiortxt9;
               }
               lpos = FIOBMAX-1;
               if(rdtxtf(*(txtfile*)&(fiocurf->ffile),fiocrec,lpos) == NULL) {
                  fioier = ERTX_EOF;
                  fioerror(0);
               }
               else {
                  fiocurf->fstat1 |= FREADLAST;
                  while(fiolcol > 0 && (fiocrec[fiolcol-1] == '\n' 
                                        || fiocrec[fiolcol-1] == 10 
                                        || fiocrec[fiolcol-1] == 13)) fiocrec[--fiolcol] = '\0';
                  fiocrec[fiolcol] = '\0';
               }
            }
            else if(fiocurf->fstat & FENDFIL) {
               fioier = ERTX_EOF;
               fioerror(0);
            }
            else if(rdtxtf(*(txtfile*)&(fiocurf->ffile),fiocrec,FIOBMAX-1) == NULL){
               fiocurf->fstat |= FENDFIL;
               fioier = ERTX_EOF;
               fioerror(0);
            }
            else {
               fiocurf->frsize += 1;
               fiolcol = strlen(fiocrec);
               while(fiolcol > 0 && (fiocrec[fiolcol-1] == '\n' 
                                     || fiocrec[fiolcol-1] == 10 
                                     || fiocrec[fiolcol-1] == 13)) fiocrec[--fiolcol] = '\0';
               if(fiocurf->fstat1 & FNEEDCRLF) fiocurf->fstat1 ^= FNEEDCRLF;
            }
         }
         else if(fiocurf->funit != INTERNAL && fiocurf->fassob == NULL) {
            if(fiocurf->frsize > 0) {
               fiolcol = (int)(fiocurf->frsize);
               cpymem(*(char**)&(fiocurf->ffile),fiocrec,fiolcol);
               *(char**)&(fiocurf->ffile) += fiolcol;
            }
            else {
               s1 = (char**)(*(char**)&(fiocurf->ffile));
               fiolcol = strlen(*s1);
               cpymem(*s1,fiocrec,fiolcol);
               *(char**)&(fiocurf->ffile) = (char*)(s1 + 1);
            }
            fiocrec[fiolcol] = '\0';
         }
         else {
            if((intu = fiocurf->fassob) == NULL) intu = *(char**)&(fiocurf->ffile);
            ipos = (int) (fiocurf->frecrd);
            lpos = (int) (fiocurf->fmaxrc);
            intu += ipos;
            if(fiocurf->fstat & FDIRECT) {
               fiolcol = (int) (fiocurf->frsize);
               if((fiolcol + ipos) > lpos) {
                  fioier = ERTX_EOF;
                  fioerror(0);
               }
               else {
                  cpymem(intu,fiocrec,fiolcol);
                  fiocurf->frecrd = ipos + fiolcol;
                  fiocrec[fiolcol] = '\0';
               }
            }
            else if(fiocurf->fstat & FENDFIL) {
               fioier = ERTX_EOF;
               fioerror(0);
            }
            else {
               fiolcol = 0;
               while((ipos < lpos) && (fiolcol < FIOBMAX) && (*intu != '\n')) {
                  fiocrec[fiolcol++] = *intu++;
                  ipos++;
               }
               if((ipos >= lpos) || (fiolcol >= FIOBMAX)) {
                  fiocurf->fstat |= FENDFIL;
                  fioier = ERTX_EOF;
                  fioerror(0);
               }
               else {
                  fiocrec[fiolcol] = '\0';
                  fiocurf->frecrd = ipos;
               }
            }
         }
      fiortxt9:
         fiolcol--;
         fioicol = 0;
      }

   /*****************************************************************************/
   /*                                                                           */
   /* FIOSPACE: Skip White Space In Record                                      */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* This function is a utility used by the list-directed input processing     */
   /* functions to skip over white space within the input record. White space   */
   /* consists of blanks, tabs, and newlines.                                   */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* If the function reads a new record during its processing, it returns a 1; */
   /* else it returns a zero.                                                   */
   /*                                                                           */
   /* See also:                                                                 */
   /*                                                                           */
   /* Referenced by:                                                            */
   /*                                                                           */
   /* Element       Description of use                                          */
   /* -------       ------------------                                          */
   /* fioffld       Get next free-form field                                    */
   /* fiordx        Read complex values                                         */
   /* fiordz        Read double complex values                                  */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* Simply increment the record control variables until a nonblank character  */
   /* is encountered.                                                           */
   /*                                                                           */
   /*****************************************************************************/

#ifdef FPROTOTYPE
   int fiospace(void)
#else
      int fiospace()
#endif
      {
         int i;

         for(i = 0; ;i = 1)
         {
            while(fiocrec[fioicol] == ' ' || fiocrec[fioicol] == '\t') fioicol++;
            if(fiocrec[fioicol]) break;
            fiortxt();
            if(fioier) break;
         }
         return i;
      }

   /*****************************************************************************/
   /*                                                                           */
   /* FIORCHK: Check Fixed-Form Input Field                                     */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* This function controls the physical reading of text records. If the       */
   /* logical end-of-record has been reached, then the next physical record     */
   /* is read. If a physical end-of-record has been reached, prior to the       */
   /* logical end-of-record, then the physical end-of-record is extended by     */
   /* padding the record with blanks.                                           */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* None, the function is void. The effect of its processing is reflected     */
   /* in the various global format control variables.                           */
   /*                                                                           */
   /* See also:                                                                 */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* Straightforward, see the description above.                               */
   /*                                                                           */
   /*****************************************************************************/

#ifdef FPROTOTYPE
   void fiorchk(int nfield)
#else
      void fiorchk(nfield)
      int nfield;
#endif
   {
      while((fioicol + nfield) > fiolcol)
      {
         fiocrec[++fiolcol] = ' ';
         if(fiolcol >= FIOBMAX)
         {
            fioier = ERCK_BUF;
            break;
         }
      }
   }

   /*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
   /*                                                                           */
   /* FIOFINP: Formatted Input                                                  */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* Performs non-variable related formatted input functions until an end-of-  */
   /* format or a variable related specifications is encountered. The actual    */
   /* operations performed by this function are as follows:                     */
   /*                                                                           */
   /*     Specification  Code Description                                       */
   /*     -------------  ---- -----------                                       */
   /*     nH               1  Display holerith string                           */
   /*     "c1..cn"         1  Display delimited string                          */
   /*     nX               2  skip right n places                               */
   /*     TRn              2  skip right n places                               */
   /*     Tn               3  move to position n                                */
   /*     TLn              4  skip left n places                                */
   /*     SS               5  set the plus sign to a space                      */
   /*     SP               6  set the plus sign to a +                          */
   /*     BN               7  set blanks to null                                */
   /*     BZ               8  set blanks to zero                                */
   /*     /                9  physically write the current line                 */
   /*     nP              10  set the floating scale factor to n                */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* None the function is void; however, the global variable "fioier" may be   */
   /* set to an error code if a problem is encountered.                         */
   /*                                                                           */
   /*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#ifdef FPROTOTYPE
   static void fiofinp(int context)
#else
      static void fiofinp(context)
      int context;                      /* Context of call 0 = value, 1 = end */
#endif
   {

      /**************************************************************************/
      /*                                                                        */
      /* Step 1: Obtain the next format specification from the generic format   */
      /* processing function and if an error occurs or if the specification is  */
      /* variable related or a rescan indication then return that specification */
      /* to the calling function for processing. Else branch to perform the     */
      /* indicated operation.                                                   */
      /*                                                                        */
      /**************************************************************************/

   fiofinp1:
      fionxtf();
      if(!fiofspec) fioblkn = 0;
      if(fioier || !fiofspec || fiofspec > 11) return;
      switch(fiofspec)
      {
      case -2:
         goto fiofinp1;
      case -1:
         if(!context) goto fiofinp1;
         break;

         /***********************************************************************/
         /*                                                                     */
         /* Step 2: A "display string" specification is to be executed. Do so,  */
         /* and loop back to step 1. In traditional FORTRAN this means to copy  */
         /* characters from the input record int the FORMAT specification.      */
         /*                                                                     */
         /***********************************************************************/

      case 1:
         fiorchk(fioiwd);
         cpymem(fiocrec+fioicol,fiosadr,fioiwd);
         fioicol += fioiwd;
         goto fiofinp1;

         /***********************************************************************/
         /*                                                                     */
         /* Step 3: A "skip right n spaces" specification is to be executed. Do */
         /* so by inserting blanks at the back of the input record if necessary */
         /* and loop back to step 1.                                            */
         /*                                                                     */
         /***********************************************************************/

      case 2:
         fiorchk(fioiwd);
         fioicol += fioiwd;
         goto fiofinp1;

         /***********************************************************************/
         /*                                                                     */
         /* Step 4: A "move to position n" specification is to be executed.     */
         /* Reset the current character position in the current text record if  */
         /* the position is to the left; else insert the needed number of       */
         /* blanks to move to the right.                                        */
         /*                                                                     */
         /* Then loop back to step 1. Note that the stated position number must */
         /* be decremented by one to compensate for the fact the FORTRAN FORMAT */
         /* assumes that chararacter number 1 is at position 1 and not 0 as in  */
         /* C.                                                                  */
         /*                                                                     */
         /***********************************************************************/

      case 3:
         fiorchk(fioiwd - fioicol);
         fioicol = fioiwd - 1;
         while(fiolcol < fioicol) fiocrec[++fiolcol] = ' ';
         goto fiofinp1;

         /***********************************************************************/
         /*                                                                     */
         /* Step 5: A "skip left n places" specification is to be executed.     */
         /* Decrement the current character position in the current text record */
         /* accordingly, making certain that the characters position does not   */
         /* go negative. Then loop back to step 1.                              */
         /*                                                                     */
         /***********************************************************************/

      case 4:
         fioicol -= fioiwd;
         if(fioicol < 0) fioicol = 0;
         goto fiofinp1;

         /***********************************************************************/
         /*                                                                     */
         /* Step 6: A "set plus sign to plus or blank" specification is to be   */
         /* executed. Since this specification has no relevance to input        */
         /* processing it is simply ignored here. One could make this an error  */
         /* by setting "fioier". Then loop back to step 1.                      */
         /*                                                                     */
         /***********************************************************************/

      case 5: case 6:
         goto fiofinp1;

         /***********************************************************************/
         /*                                                                     */
         /* Step 7: A "set blanks to null" specification is to be executed. Set */
         /* the appropriate global variables and loop back to step 1.           */
         /*                                                                     */
         /***********************************************************************/

      case 7:
         fioblkn = 1;
         goto fiofinp1;

         /***********************************************************************/
         /*                                                                     */
         /* Step 8: A "set blanks to zero" specification is to be executed. Set */
         /* the appropriate global variables and loop back to step 1.           */
         /*                                                                     */
         /***********************************************************************/

      case 8:
         fioblkn = -1;
         goto fiofinp1;

         /***********************************************************************/
         /*                                                                     */
         /* Step 9: A "physically read next record" specification is the be     */
         /* executed. Do so and loop back to step 1.                            */
         /*                                                                     */
         /***********************************************************************/

      case 9:
         fiortxt();
         goto fiofinp1;

         /***********************************************************************/
         /*                                                                     */
         /* Step 10: A "set scale factor" specification is to be executed. Set  */
         /* the global variable and loop back to step 1.                        */
         /*                                                                     */
         /***********************************************************************/

      case 10:
         fioscal = fioiwd;
         goto fiofinp1;

         /***********************************************************************/
         /*                                                                     */
         /* Step 11: An End-of-format has been encountered. If processing       */
         /* values, physically read the next record and loop back to step 1;    */
         /* else end local processing.                                          */
         /*                                                                     */
         /***********************************************************************/

      case 11:
         if(!context)
         {
            fiortxt();
            goto fiofinp1;
         }
      }
   }
   /*===========================================================================*/
   /*                                                                           */
   /* fiorbiv: FORTRAN Read Binary Values                                       */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* This function reads binary values from a file.                            */
   /*                                                                           */
   /* Argument(s)  Description of use                                           */
   /* -----------  ------------------                                           */
   /* value        Points to values being read                                  */
   /* nvalue       Number of bytes to be read                                   */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if all went well, else an error code. See the general discussion   */
   /* of the FORTRAN I/O capabilities for a listing of the possible error codes.*/
   /*                                                                           */
   /*===========================================================================*/

#ifdef FPROTOTYPE
   int fiorbiv(VOID* value,int nvalue)
#else
      int fiorbiv(value,nvalue)
      VOID* value;                      /* Points to values being read */
   int nvalue;                       /* Number of bytes to be read */
#endif
   {
      char* intu;
      LONG ipos;
      LONG lpos;

      if(fioier) goto fiorbiv9;
      if(nvalue > fiocurf->frsize) nvalue = (int)(fiocurf->frsize);
      if(nvalue)
      {
         if(!(fiocurf->fstat & FINTERNAL))
         {
            if((int)rdbinf(*(binfile*)&(fiocurf->ffile),value,nvalue) < nvalue)
            {
               fioier = ERBV_EOF;
            }
            else
            {
               if(!(fiocurf->fstat & FDIRECT)) fiocurf->frsize -= nvalue;
               else fiocurf->fstat1 |= FREADLAST;
            }
         }
         else
         {
            if((intu = fiocurf->fassob) == NULL)
               intu = *(char**)&(fiocurf->ffile);
            ipos = fiocurf->frecrd;
            lpos = fiocurf->fmaxrc;
            if((nvalue + ipos) > lpos) fioier = ERBV_EOF;
            else
            {
               cpymem(intu+ipos,value,nvalue);
               fiocurf->frecrd = ipos + nvalue;
               if(!(fiocurf->fstat & FDIRECT)) fiocurf->frsize -= nvalue;
            }
         }
      }
   fiorbiv9:
      return fioier;
   }

   /*****************************************************************************/
   /*                                                                           */
   /* FIOFFLD: Get Next Free-Form Field                                         */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* This function locates the start of the next unit of information when      */
   /* free-form reads are being performed. Its skips over blanks and commas.    */
   /* If an end-of-record is encountered, then it reads the next physical       */
   /* record from the current file. The major complexity in this routine has    */
   /* to do with repeat counts and null-values. Note that when a "/" character  */
   /* is encountered, then all remaining values are set to null.                */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* The function returns a 1 if there is a nonnull value to be read. In this  */
   /* case, "fioicol" points to the first significant character of this value.  */
   /* A zero is returned, if a null value is to be read. From the standpoint of */
   /* the calling function, this means that the corresponding input value is to */
   /* remain unchanged.                                                         */
   /*                                                                           */
   /* See also:                                                                 */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* See detailed steps below.                                                 */
   /*                                                                           */
   /*****************************************************************************/

#ifdef FPROTOTYPE
   int fioffld(void)
#else
      int fioffld()
#endif
      {
         int ic;
         static int savcol = 0;         /* Repeat group starting column */
         static int nulval = 0;  /* Repeat is a null value indicator */

         /**************************************************************************/
         /*                                                                        */
         /* Step 1: If there is a repeat count active, then reset the current      */
         /* input pointer and end local processing.                                */
         /*                                                                        */
         /**************************************************************************/

         if(fiorpeat > 0 && fiocrec[0] == '\0')
         {
            fiorpeat--;
            fioicol = savcol + 1;
            return nulval;
         }
         fiorpeat = 0;

         /**************************************************************************/
         /*                                                                        */
         /* Step 2: Scan through the input stream until a valid input character is */
         /* encountered.                                                           */
         /*                                                                        */
         /**************************************************************************/

         if(fioicol != 0) ic = 0;
         else ic = 1;
         ic |= fiospace();
         if(fioier) return 0;
         if(fiocrec[fioicol] == ',')
         {
            fioicol++;
            if(ic) return 0;
            fiospace();
            if(fioier) return 0;
            if(fiocrec[fioicol] == ',') return 0;
         }
         else if(fiocrec[fioicol] == '/')
         {
            fiorpeat = 32765;
            savcol = fioicol;
            fiocrec[0] = '\0';
            nulval = 0;
            return 0;
         }

         /**************************************************************************/
         /*                                                                        */
         /* Step 3: If the current input token is not a repeat count, end local    */
         /* processing; else establish the repeat control variables.               */
         /*                                                                        */
         /**************************************************************************/

         for(ic = fioicol; ic <= fiolcol; ic++)
         {
            if(!(ChrTypeFlags[(int)fiocrec[ic]] & CNUMBER)) break;
            fiorpeat = fiorpeat * 10 + ChrInformation[(int)fiocrec[ic]];
         }
         if(ic < fiolcol && fiorpeat > 0 && fiocrec[ic] == '*')
         {
            fiorpeat--;
            savcol = ic;
            fiocrec[0] = '\0';
            fioicol = ic + 1;
            if(fiocrec[fioicol] == ' ' ||  fiocrec[fioicol] == ',') nulval = 0;
            else nulval = 1;
            return nulval;
         }
         else fiorpeat = 0;
         return 1;
      }

   /*****************************************************************************/
   /*                                                                           */
   /* FTNXCONS: FORTRAN Exact Representation Constant                           */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* Octal and hexidecimal character strings are physically stored as          */
   /* character strings and are treated as "typeless" numeric constants. This   */
   /* function performs this conversion. The parameter "base" specifies the     */
   /* base of the exact representation constant. It may have a value of 8 or 16 */
   /* only.The "xcons" parameter points to the actual string for the constant.  */
   /* The "Xlen" parameter specifies the length of the string; while the "vlen" */
   /* parameter specifies the length of the resultant value. It has a maximum   */
   /* setting or 32.                                                            */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A pointer to the result of the conversion. This pointer must always be    */
   /* cast a pointer to the type of the desired value.                          */
   /*                                                                           */
   /* See also:                                                                 */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* See detailed steps below.                                                 */
   /*                                                                           */
   /*****************************************************************************/

#ifdef FPROTOTYPE
   char* ftnxcons(int base,char* xcons,int xlen,int vlen)
#else
      char* ftnxcons(base,xcons,xlen,vlen)
      int base;                      /* Base of exact representation constant */
   char* xcons;                   /* String form of exact representation */
   int xlen;                      /* Length of exact representation string */
   int vlen;                      /* Length of the resultant value */
#endif
   {
      static char ival[32];
      int i;
      int minbit;
      int ic;
      int j;
      int vbit;
      unsigned char uc;

      /*------------------------------------------------------------------------*/
      /*                                                                        */
      /* Step 1: Initialize the return value to show all zero bytes.            */
      /*                                                                        */
      /*------------------------------------------------------------------------*/

      filmem(ival,vlen,'\0');

      /*------------------------------------------------------------------------*/
      /*                                                                        */
      /* Step 2: Scan forward in the exact representation constant until a      */
      /* nonblank character is found. If no nonblank character is found, end    */
      /* local processing.                                                      */
      /*                                                                        */
      /*------------------------------------------------------------------------*/

      for(i = 0; i < xlen; i++) if(xcons[i] != ' ') break;
      if(i >= xlen || xcons[i] == '\0') goto xcons9;

      /*------------------------------------------------------------------------*/
      /*                                                                        */
      /* Step 3: Determine the number of digits in the constant, by scanning    */
      /* forward to the end of the string or until a blank or null character is */
      /* encountered.                                                           */
      /*                                                                        */
      /*------------------------------------------------------------------------*/

      xcons += i;
      xlen -= i;
      for(i = 0; i < xlen; i++) if(xcons[i] == ' ' || xcons[i] == '\0') break;
      xlen = i;

      /*------------------------------------------------------------------------*/
      /*                                                                        */
      /* Step 4: Compute the compute the minimumn bit number of the digit value */
      /* and the number of bits in the typeless result.                         */
      /*                                                                        */
      /*------------------------------------------------------------------------*/

      if(base == 8) minbit = 5;
      else minbit = 4;
      vbit = 8*vlen;

      /*------------------------------------------------------------------------*/
      /*                                                                        */
      /* Step 5: Move through the digits from the back to the front and place   */
      /* the bit values into the result vector.                                 */
      /*                                                                        */
      /*------------------------------------------------------------------------*/

      for(i = xlen-1; i >= 0; i--)
      {
         ic = xcons[i];
         if(ChrTypeFlags[ic] & CNUMBER) ic = ChrInformation[ic];
         else if(ChrTypeFlags[ic] & CUPPERC)
            ic = ChrInformation[ChrInformation[ic]] + 9;
         else if(ChrTypeFlags[ic] & CLOWERC) ic = ChrInformation[ic] + 9;
         else ic = 0;
         if(ic >= base)
         {
            if(base == 2) fioier = EXCN_ICB;
            else if(base == 8) fioier = EXCN_ICO;
            else fioier = EXCN_ICH;
            fioerc = xcons[i];
            ic = 0;
         }
         uc = (unsigned char)(ic);
         for(j = 8; j > minbit; j--)
         {
            fifibit((unsigned char*)ival,vbit,fifxbit(&uc,j));
            vbit--;
            if(vbit == 0) goto xcons9;
         }
      }
   xcons9:
      if(fiobsex) fifrbit((unsigned char*)ival,vlen);
      return ival;
   }

   /*****************************************************************************/
   /*                                                                           */
   /* FIORALPH: Read Alphabetic Information                                     */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* This function blank fills a character string and then reads characters    */
   /* from the current input record into the string. Read characters are left-  */
   /* justified in the the string. An characters beyond the end of the string   */
   /* are discarded. If input is free-form and if the characters read begin     */
   /* with a single quote, then the material within the quoted list is entered  */
   /* into the character string, with '' reducing to a single quote.            */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* None, the function is void. The effect of its processing is reflected in  */
   /* the various global format control variables.                              */
   /*                                                                           */
   /* See also:                                                                 */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* Straightforward, see description above.                                   */
   /*                                                                           */
   /*****************************************************************************/

#ifdef FPROTOTYPE
   void fioralph(char* alpha,int nalpha,int nfield)
#else
      void fioralph(alpha,nalpha,nfield)
      char* alpha;                      /* Character string */
   int nalpha;                       /* Length of string */
   int nfield;                       /* Width of field */
#endif
   {
      int ic;
      int nc;
      int iquote;

      if(fiofspec == 0 && (fiocrec[fioicol] == '\'' || fiocrec[fioicol] == '"'))
      {
         nc = 0;
         iquote = fiocrec[fioicol];
         for(;;)
         {
            fioicol++;
            if(fioicol > fiolcol)
            {
               fiortxt();
               if(fioier) break;
            }
            if((fiocrec[fioicol] == '\\') && 
               (ChrTypeFlags[(int)fiocrec[fioicol+1]] & CNUMBER))
            {
               ic = ChrInformation[(int)fiocrec[fioicol+1]];
               fioicol++;
               while(ChrTypeFlags[(int)fiocrec[fioicol+1]] & CNUMBER)
               {
                  ic = ic * 8 + ChrInformation[(int)fiocrec[fioicol+1]];
                  fioicol++;
               }
               if(nc < nalpha) *(alpha+nc++) = (char) ic;
               continue;
            }
            if(fiocrec[fioicol] == iquote)
            {
               if(fiocrec[fioicol+1] != iquote) break;
               fioicol++;
            }
            if(nc < nalpha) *(alpha+nc++) = fiocrec[fioicol];
         }
         if(nc < nalpha) filmem(alpha+nc,nalpha-nc,' ');
         fioicol++;
      }
      else
      {
         fiorchk(nfield);
         if(fiondec == 0)
         {
            nc = nalpha;
            if(nc > nfield)
            {
               nc = nfield;
               filmem(alpha+nc,nalpha-nc,' ');
            }
            else if(nc < nfield)
            {
               fioicol += (nfield-nc);
               nfield = nc;
            }
            cpymem(fiocrec+fioicol,alpha,nc);
         }
         else if(fiondec == -1) 
            cpymem(ftnxcons(8,fiocrec+fioicol,nfield,nalpha),alpha,nalpha);
         else 
            cpymem(ftnxcons(16,fiocrec+fioicol,nfield,nalpha),alpha,nalpha);
         fioicol += nfield;
      }
   }

   /*===========================================================================*/
   /*                                                                           */
   /* fiordb: Read FORTRAN Boolean Vector                                       */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* Reads a vector of boolean (short logical) values from the current input   */
   /* file in accordance with the current format specification. Note that in    */
   /* this function each individual boolean value is assumed to have its own    */
   /* corresponding format specification if a formatted read is being           */
   /* performed.                                                                */
   /*                                                                           */
   /* Argument(s)  Description of use                                           */
   /* -----------  ------------------                                           */
   /* bool         The values to be read                                        */
   /* nval         The number of values to be read                              */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if there is no error flag set, else an error code. See the general */
   /* discussion if FORTRAN I/O capabilities for a listing of the possible      */
   /* error codes.                                                              */
   /*                                                                           */
   /*===========================================================================*/

#ifdef FPROTOTYPE
   int fiordb(unsigned short* mybool ,int nval)
#else
      int fiordb(mybool,nval)
      unsigned short* mybool;             /* The values to be read */
   int nval;                         /* The number of values to be read */
#endif
   {
      int i;                    /* Dummy counter through values */
      int nc;                   /* Number of characters in field */
      unsigned short bv;        /* Contains the current value */

      /**************************************************************************/
      /*                                                                        */
      /* Step 1: Move through the boolean values one at a time until all have   */
      /* been read or until an error is encountered. Note that if the error     */
      /* flag is already set when this function is called, then it will exit    */
      /* immediately.                                                           */
      /*                                                                        */
      /**************************************************************************/

      for(i = 0; (i < nval) && !fioier; i++, mybool++)
      {

         /***********************************************************************/
         /*                                                                     */
         /* Step 2: Obtain the format specifications until a variable related   */
         /* one is encountered.                                                 */
         /*                                                                     */
         /***********************************************************************/

         fiofinp(0);
         if(fioier) goto fiordb9;

         /***********************************************************************/
         /*                                                                     */
         /* Step 3: If FORTRAN 66 type alphabetic information is being read     */
         /* into the variable, accept it and loop back to step 1. If you wish   */
         /* to inpose strict FORTRAN 77 type I/O then comment out the following */
         /* statement and the "else" logic following it.                        */
         /*                                                                     */
         /***********************************************************************/

         if(fiofspec == 12) fioralph((char*)mybool,sizeof(short),fioiwd);
         else
         {
            /********************************************************************/
            /*                                                                  */
            /* Step 4: If free-field input is being performed, read the next    */
            /* character from the input stream using free-form conventions. If  */
            /* it is a 'T' or 't' then set the value to true, else set it to    */
            /* FALSE. Then loop back to step 1.                                 */
            /*                                                                  */
            /********************************************************************/

            bv = 0;
            if(fiofspec == 0)
            {
               if(fioffld())
               {
                  if(fiocrec[fioicol] == '.') fioicol++;
                  if(fiocrec[fioicol] == 't' || fiocrec[fioicol] == 'T')
                     bv = 1;
                  do
                  {
                     i = fiocrec[++fioicol];
                  }
                  while(i != ',' && i != ' ' && i != '\0' && i != '/');
               }
            }

            /********************************************************************/
            /*                                                                  */
            /* Step 5: An invalid format specification has been found. Set the  */
            /* error code accordingly and loop back to step 1.                  */
            /*                                                                  */
            /********************************************************************/

            else if(fiofspec != 13) fioier = ERDB_IFS;

            /********************************************************************/
            /*                                                                  */
            /* Step 6: If an "L" format specification is encountered, determine */
            /* the field width and read the first nonblank character. If it is  */
            /* a 'T' or 't' then set the value to true, else set it to FALSE.   */
            /* Then loop back to step 1.                                        */
            /*                                                                  */
            /********************************************************************/

            else
            {
               if(fioiwd == 0) fioiwd = 16;
               fiorchk(fioiwd);
               nc = fioicol;
               fioicol += fioiwd;
               while(nc < fioicol && fiocrec[nc] == ' ') nc++;
               if(nc < (fioicol-1) && fiocrec[nc] == '.') nc++;
               if(nc < fioicol && (fiocrec[nc] == 't' || fiocrec[nc] == 'T'))
                  bv = 1;
            }
#ifdef ISUNALIGNED
            put16u((char*)(mybool),bv);
#else
            *mybool = bv;
#endif
         }
      }
      /**************************************************************************/
      /*                                                                        */
      /* Step 7: Either all boolean values have been read or an error has       */
      /* occurred. Check for an error and if one is present perform the user    */
      /* specified action.                                                      */
      /*                                                                        */
      /**************************************************************************/

   fiordb9:
      return fioerror(0);
   }
   /*===========================================================================*/
   /*                                                                           */
   /* fiordc: Read FORTRAN Charactor Value Vector                               */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* Reads a vector of charaxter values from the current input file in         */
   /* accordance with the current format specification. Each value is assumed   */
   /* to have its own corresponding format specification if a formatted read is */
   /* being performed.                                                          */
   /*                                                                           */
   /* Argument(s)  Description of use                                           */
   /* -----------  ------------------                                           */
   /* value        Vector of values                                             */
   /* nval         Number of values to be read                                  */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if there is no error flag set, else an error code. See the general */
   /* discussion if FORTRAN I/O capabilities for a listing of the possible      */
   /* error codes.                                                              */
   /*                                                                           */
   /*===========================================================================*/

#ifdef FPROTOTYPE
   int fiordc(char* value,int nval)
#else
      int fiordc(value,nval)
      char* value;                      /* Vector of values */
   int nval;                         /* Number of values to be read */
#endif
   {
      int i;                    /* Dummy counter through values */

      /**************************************************************************/
      /*                                                                        */
      /* Step 1: Move through the values one at a time until all have been read */
      /* or until an error is encountered. Note that if the error flag is       */
      /* already set when this function is called, then it will exit            */
      /* immediately.                                                           */
      /*                                                                        */
      /**************************************************************************/

      for(i = 0; (i < nval) && !fioier; i++,value++)
      {

         /***********************************************************************/
         /*                                                                     */
         /* Step 2: Obtain the format specifications until a variable related   */
         /* one is encountered.                                                 */
         /*                                                                     */
         /***********************************************************************/

         fiofinp(0);
         if(fioier) goto fiordc9;

         /***********************************************************************/
         /*                                                                     */
         /* Step 3: If free-field input is being processed, get the start of    */
         /* the next input field and compute its value, then loop back to step  */
         /* 1.                                                                  */
         /*                                                                     */
         /***********************************************************************/

         if(fiofspec == 0)
         {
            if(fioffld())
            {
               *value = (char) fiostod(fiocrec+fioicol,fiolcol-fioicol+1);
               if(fioerc) fioicol += (fioerc-1);
               else fioicol = fiolcol + 1;
            }
         }

         /***********************************************************************/
         /*                                                                     */
         /* Step 4: If FORTRAN 66 type alphabetic information is being read     */
         /* into the variable, accept it and loop back to step 1. If you wish   */
         /* to impose strict FORTRAN 77 type I/O then comment out the following */
         /* statement.                                                          */
         /*                                                                     */
         /***********************************************************************/

         else if(fiofspec == 12) fioralph(value,sizeof(char),fioiwd);

         /***********************************************************************/
         /*                                                                     */
         /* Step 5: If an "I" format specification is encountered, make certain */
         /* that a fixed field is there and do the conversion. Upon completion, */
         /* loop back to step 1.                                                */
         /*                                                                     */
         /***********************************************************************/

         else if(fiofspec == 14)
         {
            if(fioiwd == 0) fioiwd = 16;
            fiorchk(fioiwd);
            fiondec = 0;
            *value = (char) fiostod(fiocrec+fioicol,fioiwd);
            if(fioerc) fioicol += fioerc;
            else fioicol += fioiwd;
         }

         /***********************************************************************/
         /*                                                                     */
         /* Step 6: A "Q" format specification has been entered which requests  */
         /* the length of the remaining input record.                           */
         /*                                                                     */
         /***********************************************************************/

         else if(fiofspec == 20) *value = (char)(fiolcol - fioicol + 1);

         /***********************************************************************/
         /*                                                                     */
         /* Step 7: An invalid specification has been encountered. Set the      */
         /* error flag accordingly and loop back to step 1.                     */
         /*                                                                     */
         /***********************************************************************/

         else fioier = ERDI_IFS;
      }
      /**************************************************************************/
      /*                                                                        */
      /* Step 7: Either all values have been read or an error has occurred.     */
      /* Check for an error and if one is present perform the user specified    */
      /* action.                                                                */
      /*                                                                        */
      /**************************************************************************/

   fiordc9:
      return fioerror(0);
   }
   /*===========================================================================*/
   /*                                                                           */
   /* fiordd: Read FORTRAN Double Precision Vector                              */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* Reads a vector of double precision floating point values from the current */
   /* input file in accordance with the current format specification. Each      */
   /* value is assumed to have its own corresponding format specification if a  */
   /* formatted read is being performed.                                        */
   /*                                                                           */
   /* Argument(s)  Description of use                                           */
   /* -----------  ------------------                                           */
   /* value        Points to values to be read                                  */
   /* nval         Number of values to be read                                  */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if there is no error flag set, else an error code. See the general */
   /* discussion if FORTRAN I/O capabilities for a listing of the possible      */
   /* error codes.                                                              */
   /*                                                                           */
   /*===========================================================================*/

#ifdef FPROTOTYPE
   int fiordd(double* value,int nval)
#else
      int fiordd(value,nval)
      double* value;                    /* Points to values to be read */
   int nval;                         /* Number of values to be read */
#endif
   {
      int i;                    /* Dummy counter through values */
      double temp;              /* Temporary value storage */

      /**************************************************************************/
      /*                                                                        */
      /* Step 1: Move through the values one at a time until all have been read */
      /* or until an error is encountered. Note that if the error flag is       */
      /* already set when this function is called, then it will exit            */
      /* immediately.                                                           */
      /*                                                                        */
      /**************************************************************************/

      for(i = 0; (i < nval) && !fioier; i++,value++)
      {

         /***********************************************************************/
         /*                                                                     */
         /* Step 2: Obtain the format specifications until a variable related   */
         /* one is encountered.                                                 */
         /*                                                                     */
         /***********************************************************************/

         fiofinp(0);
         if(fioier)goto fiordd9;

         /***********************************************************************/
         /*                                                                     */
         /* Step 3: If free-field input is being processed, get the start of    */
         /* the next input field and compute its value, then loop back to step  */
         /* 1.                                                                  */
         /*                                                                     */
         /***********************************************************************/

         if(fiofspec == 0)
         {
            if(fioffld())
            {
               temp = fiostod(fiocrec+fioicol,fiolcol-fioicol+1);
#ifdef ISUNALIGNED
               if(!fioier) putdbl((char*)(value),temp);
#else
               if(!fioier) *value = temp;
#endif
               if(fioerc) fioicol += (fioerc-1);
               else fioicol = fiolcol + 1;
            }
         }

         /***********************************************************************/
         /*                                                                     */
         /* Step 4: If FORTRAN 66 type alphabetic information is being read     */
         /* into the variable, accept it and loop back to step 1. If you wish   */
         /* to inpose strict FORTRAN 77 type I/O then comment out the following */
         /* statement.                                                          */
         /*                                                                     */
         /***********************************************************************/

         else if(fiofspec == 12) fioralph((char*)value,sizeof(double),fioiwd);

         /***********************************************************************/
         /*                                                                     */
         /* Step 5: If any one of the floating point specifications -- F, D, E, */
         /* G -- is encountered, make certain that a fixed field is there and   */
         /* do the conversion. Note that the code is not nit-picking here about */
         /* distinguishing the different format types. Upon completion, loop    */
         /* back to step 1.                                                     */
         /*                                                                     */
         /***********************************************************************/

         else if(fiofspec > 14)
         {
            if(fioiwd == 0) fioiwd = 16;
            fiorchk(fioiwd);
            temp = fiostod(fiocrec+fioicol,fioiwd);
            if(!(ChrFlags & CHR_HASEXPON) && fioscal && (fiofspec > 14))
               temp /= pow(10.0,(double)fioscal);
#ifdef ISUNALIGNED
            if(!fioier) putdbl((char*)(value),temp);
#else
            if(!fioier) *value = temp;
#endif
            if(fioerc) fioicol += fioerc;
            else fioicol += fioiwd;
         }

         /***********************************************************************/
         /*                                                                     */
         /* Step 6: An invalid specification has been encountered. Set the      */
         /* error flag accordingly and loop back to step 1.                     */
         /*                                                                     */
         /***********************************************************************/

         else fioier = ERDD_IFS;
      }

      /**************************************************************************/
      /*                                                                        */
      /* Step 7: Either all values have been read or an error has occurred.     */
      /* Check for an error and if one is present perform the user specified    */
      /* action.                                                                */
      /*                                                                        */
      /**************************************************************************/
   fiordd9:
      return fioerror(0);
   }
   /*===========================================================================*/
   /*                                                                           */
   /* fiordf: Read FORTRAN Floating Point Values                                */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* Reads a vector of single precision floating point values from the current */
   /* input file in accordance with the current format specification. Each      */
   /* value is assumed to have its own corresponding format specification if a  */
   /* formatted read is being performed.                                        */
   /*                                                                           */
   /* Argument(s)  Description of use                                           */
   /* -----------  ------------------                                           */
   /* value        Points to values to be read                                  */
   /* nval         Number of values to be read                                  */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if there is no error flag set, else an error code. See the general */
   /* discussion if FORTRAN I/O capabilities for a listing of the possible      */
   /* error codes.                                                              */
   /*                                                                           */
   /*===========================================================================*/

#ifdef FPROTOTYPE
   int fiordf(float* value,int nval)
#else
      int fiordf(value,nval)
      float* value;                     /* Points to values to be read */
   int nval;                         /* Number of values to be read */
#endif
   {
      int i;
      float temp;

      /**************************************************************************/
      /*                                                                        */
      /* Step 1: Move through the values one at a time until all have been read */
      /* or until an error is encountered. Note that if the error flag is       */
      /* already set when this function is called, then it will exit            */
      /* immediately.                                                           */
      /*                                                                        */
      /**************************************************************************/

      for(i = 0; (i < nval) && !fioier; i++,value++)
      {

         /***********************************************************************/
         /*                                                                     */
         /* Step 2: Obtain the format specifications until a variable related   */
         /* one is encountered.                                                 */
         /*                                                                     */
         /***********************************************************************/

         fiofinp(0);
         if(fioier) goto fiordf9;

         /***********************************************************************/
         /*                                                                     */
         /* Step 3: If free-field input is being processed, get the start of    */
         /* the next input field and compute its value, then loop back to step  */
         /* 1.                                                                  */
         /*                                                                     */
         /***********************************************************************/

         if(fiofspec == 0)
         {
            if(fioffld())
            {
               temp = (float)(fiostod(fiocrec+fioicol,fiolcol-fioicol+1));
#ifdef ISUNALIGNED
               if(!fioier) putflt((char*)(value),temp);
#else
               if(!fioier) *value  = temp;
#endif
               if(fioerc) fioicol += (fioerc-1);
               else fioicol = fiolcol + 1;
            }
         }

         /***********************************************************************/
         /*                                                                     */
         /* Step 4: If FORTRAN 66 type alphabetic information is being read     */
         /* into the variable, accept it and loop back to step 1. If you wish   */
         /* to inpose strict FORTRAN 77 type I/O then comment out the following */
         /* statement.                                                          */
         /*                                                                     */
         /***********************************************************************/

         else if(fiofspec == 12) fioralph((char*)value,sizeof(float),fioiwd);

         /***********************************************************************/
         /*                                                                     */
         /* Step 5: If any one of the floating point specifications -- I, F, D, */
         /* E, G -- is encountered, make certain that a fixed field is there    */
         /* and do the conversion. Note that the code is not nit-picking here   */
         /* about distinguishing the different format types. Upon completion,   */
         /* loop back to step 1.                                                */
         /*                                                                     */
         /***********************************************************************/

         else if(fiofspec > 13)
         {
            if(fioiwd == 0) fioiwd = 16;
            fiorchk(fioiwd);
            temp = (float)(fiostod(fiocrec+fioicol,fioiwd));
            if(!(ChrFlags & CHR_HASEXPON) && fioscal && (fiofspec > 14))
               temp /= (float)(pow(10.0,(double)fioscal));
#ifdef ISUNALIGNED
            if(!fioier) putflt((char*)(value),temp);
#else
            if(!fioier) *value = temp;
#endif
            if(fioerc) fioicol += fioerc;
            else fioicol += fioiwd;
         }

         /***********************************************************************/
         /*                                                                     */
         /* Step 6: An invalid specification has been encountered. Set the      */
         /* error flag accordingly and loop back to step 1.                     */
         /*                                                                     */
         /***********************************************************************/

         else fioier = ERDF_IFS;
      }

      /**************************************************************************/
      /*                                                                        */
      /* Step 7: Either all values have been read or an error has occurred.     */
      /* Check for an error and if one is present perform the user specified    */
      /* action.                                                                */
      /*                                                                        */
      /**************************************************************************/

   fiordf9:
      return fioerror(0);
   }
   /*===========================================================================*/
   /*                                                                           */
   /* fiordi: Read FORTRAN Short Integer Vector                                 */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* Reads a vector of short fixed point values from the current input file    */
   /* in accordance with the current format specification. Each value is        */
   /* assumed to have its own corresponding format specification if a formatted */
   /* read is being performed.                                                  */
   /*                                                                           */
   /* Argument(s)  Description of use                                           */
   /* -----------  ------------------                                           */
   /* value        Vector of values                                             */
   /* nval         Number of values to be read                                  */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if there is no error flag set, else an error code. See the general */
   /* discussion if FORTRAN I/O capabilities for a listing of the possible      */
   /* error codes.                                                              */
   /*                                                                           */
   /*===========================================================================*/

#ifdef FPROTOTYPE
   int fiordi(short* value,int nval)
#else
      int fiordi(value,nval)
      short* value;                     /* Vector of values */
   int nval;                         /* Number of values to be read */
#endif
   {
      int i;                    /* Dummy counter through values */
      short sv;                 /* The short value read */

      /**************************************************************************/
      /*                                                                        */
      /* Step 1: Move through the values one at a time until all have been read */
      /* or until an error is encountered. Note that if the error flag is       */
      /* already set when this function is called, then it will exit            */
      /* immediately.                                                           */
      /*                                                                        */
      /**************************************************************************/

      for(i = 0; (i < nval) && !fioier; i++,value++)
      {

         /***********************************************************************/
         /*                                                                     */
         /* Step 2: Obtain the format specifications until a variable related   */
         /* one is encountered.                                                 */
         /*                                                                     */
         /***********************************************************************/

         fiofinp(0);
         if(fioier) goto fiordi9;

         /***********************************************************************/
         /*                                                                     */
         /* Step 3: If free-field input is being processed, get the start of    */
         /* the next input field and compute its value, then loop back to step  */
         /* 1.                                                                  */
         /*                                                                     */
         /***********************************************************************/

         if(fiofspec == 0)
         {
            if(fioffld())
            {
               sv = (short) fiostod(fiocrec+fioicol,fiolcol-fioicol+1);
#ifdef ISUNALIGNED
               put16s((char*)(value),sv);
#else
               *value = sv;
#endif
               if(fioerc) fioicol += (fioerc-1);
               else fioicol = fiolcol + 1;
            }
         }

         /***********************************************************************/
         /*                                                                     */
         /* Step 4: If FORTRAN 66 type alphabetic information is being read     */
         /* into the variable, accept it and loop back to step 1. If you wish   */
         /* to impose strict FORTRAN 77 type I/O then comment out the following */
         /* statement.                                                          */
         /*                                                                     */
         /***********************************************************************/

         else if(fiofspec == 12)fioralph((char*)value,sizeof(short),fioiwd);

         /***********************************************************************/
         /*                                                                     */
         /* Step 5: If an "I" format specification is encountered, make certain */
         /* that a fixed field is there and do the conversion. Upon completion, */
         /* loop back to step 1.                                                */
         /*                                                                     */
         /***********************************************************************/

         else if(fiofspec == 14)
         {
            if(fioiwd == 0) fioiwd = 16;
            fiorchk(fioiwd);
            fiondec = 0;
            sv = (short) fiostod(fiocrec+fioicol,fioiwd);
#ifdef ISUNALIGNED
            put16s((char*)(value),sv);
#else
            *value = sv;
#endif
            if(fioerc) fioicol += fioerc;
            else fioicol += fioiwd;
         }

         /***********************************************************************/
         /*                                                                     */
         /* Step 6: A "Q" format specification has been entered which requests  */
         /* the length of the remaining input record.                           */
         /*                                                                     */
         /***********************************************************************/

         else if(fiofspec == 20)
         {
            sv = (short)(fiolcol - fioicol + 1);
#ifdef ISUNALIGNED
            put16s((char*)(value),sv);
#else
            *value = sv;
#endif
         }

         /***********************************************************************/
         /*                                                                     */
         /* Step 7: An invalid specification has been encountered. Set the      */
         /* error flag accordingly and loop back to step 1.                     */
         /*                                                                     */
         /***********************************************************************/

         else fioier = ERDI_IFS;
      }

      /**************************************************************************/
      /*                                                                        */
      /* Step 8: Either all values have been read or an error has occurred.     */
      /* Check for an error and if one is present perform the user specified    */
      /* action.                                                                */
      /*                                                                        */
      /**************************************************************************/

   fiordi9:
      return fioerror(0);
   }
   /*===========================================================================*/
   /*                                                                           */
   /* fiordl: Read FORTRAN Long Integer Vector                                  */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* Reads a vector of long fixed point values from the current input file in  */
   /* accordance with the current format specification. Each value is assumed   */
   /* to have its own corresponding format specification if a formatted read is */
   /* being performed.                                                          */
   /*                                                                           */
   /* Argument(s)  Description of use                                           */
   /* -----------  ------------------                                           */
   /* value        Vector of values                                             */
   /* nval         Number of values to be read                                  */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if there is no error flag set, else an error code. See the general */
   /* discussion if FORTRAN I/O capabilities for a listing of the possible      */
   /* error codes.                                                              */
   /*                                                                           */
   /*===========================================================================*/

#ifdef FPROTOTYPE
   int fiordl(LONG* value,int nval)
#else
      int fiordl(value,nval)
      LONG* value;                      /* Vector of values */
   int nval;                         /* Number of values to be read */
#endif
   {
      int i;                    /* Dummy counter through values */
      LONG lv;                  /* The long value read */

      /**************************************************************************/
      /*                                                                        */
      /* Step 1: Move through the values one at a time until all have been read */
      /* or until an error is encountered. Note that if the error flag is       */
      /* already set when this function is called, then it will exit            */
      /* immediately.                                                           */
      /*                                                                        */
      /**************************************************************************/

      for(i = 0; (i < nval) && !fioier; i++,value++)
      {

         /***********************************************************************/
         /*                                                                     */
         /* Step 2: Obtain the format specifications until a variable related   */
         /* one is encountered.                                                 */
         /*                                                                     */
         /***********************************************************************/

         fiofinp(0);
         if(fioier) goto fiordl9;

         /***********************************************************************/
         /*                                                                     */
         /* Step 3: If free-field input is being processed, get the start of    */
         /* the next input field and compute its value, then loop back to step  */
         /* 1.                                                                  */
         /*                                                                     */
         /***********************************************************************/

         if(fiofspec == 0)
         {
            if(fioffld())
            {
               lv = (LONG)fiostod(fiocrec+fioicol,fiolcol-fioicol+1);
#ifdef ISUNALIGNED
               put32s((char*)(value),lv);
#else
               *value = lv;
#endif
               if(fioerc) fioicol += (fioerc-1);
               else fioicol = fiolcol + 1;
            }
         }

         /***********************************************************************/
         /*                                                                     */
         /* Step 4: If FORTRAN 66 type alphabetic information is being read     */
         /* into the variable, accept it and loop back to step 1. If you wish   */
         /* to impose strict FORTRAN 77 type I/O then comment out the following */
         /* statement.                                                          */
         /*                                                                     */
         /***********************************************************************/

         else if(fiofspec == 12) fioralph((char*)value,sizeof(LONG),fioiwd);

         /***********************************************************************/
         /*                                                                     */
         /* Step 5: If an "I" format specification is encountered, make certain */
         /* that a fixed field is there and do the conversion. Upon completion, */
         /* loop back to step 1.                                                */
         /*                                                                     */
         /***********************************************************************/

         else if(fiofspec == 14)
         {
            if(fioiwd == 0) fioiwd = 16;
            fiorchk(fioiwd);
            fiondec = 0;
            lv = (LONG) fiostod(fiocrec+fioicol,fioiwd);
#ifdef ISUNALIGNED
            put32s((char*)(value),lv);
#else
            *value = lv;
#endif
            if(fioerc) fioicol += fioerc;
            else fioicol += fioiwd;
         }

         /***********************************************************************/
         /*                                                                     */
         /* Step 6: A "Q" format specification has been entered which requests  */
         /* the length of the remaining input record.                           */
         /*                                                                     */
         /***********************************************************************/

         else if(fiofspec == 20)
         {
            lv = fiolcol - fioicol + 1;
#ifdef ISUNALIGNED
            put32s((char*)(value),lv);
#else
            *value = lv;
#endif
         }

         /***********************************************************************/
         /*                                                                     */
         /* Step 7: An invalid specification has been encountered. Set the      */
         /* error flag accordingly and loop back to step 1.                     */
         /*                                                                     */
         /***********************************************************************/

         else fioier = ERDL_IFS;
      }

      /**************************************************************************/
      /*                                                                        */
      /* Step 8: Either all values have been read or an error has occurred.     */
      /* Check for an error and if one is present perform the user specified    */
      /* action.                                                                */
      /*                                                                        */
      /**************************************************************************/

   fiordl9:
      return fioerror(0);
   }

#ifdef FPROTOTYPE
   int fiordll(longlong* value,int nval)
#else
      int fiordll(value,nval)
      longlong* value;                  /* Vector of values */
   int nval;                         /* Number of values to be read */
#endif
   {
      int i;                    /* Dummy counter through values */
      longlong lv;              /* The long value read */

      /**************************************************************************/
      /*                                                                        */
      /* Step 1: Move through the values one at a time until all have been read */
      /* or until an error is encountered. Note that if the error flag is       */
      /* already set when this function is called, then it will exit            */
      /* immediately.                                                           */
      /*                                                                        */
      /**************************************************************************/

      for(i = 0; (i < nval) && !fioier; i++,value++)
      {

         /***********************************************************************/
         /*                                                                     */
         /* Step 2: Obtain the format specifications until a variable related   */
         /* one is encountered.                                                 */
         /*                                                                     */
         /***********************************************************************/

         fiofinp(0);
         if(fioier) goto fiordl9;

         /***********************************************************************/
         /*                                                                     */
         /* Step 3: If free-field input is being processed, get the start of    */
         /* the next input field and compute its value, then loop back to step  */
         /* 1.                                                                  */
         /*                                                                     */
         /***********************************************************************/

         if(fiofspec == 0)
         {
            if(fioffld())
            {
               lv = (longlong)(fiostod(fiocrec+fioicol,fiolcol-fioicol+1));
               *value = lv;
               if(fioerc) fioicol += (fioerc-1);
               else fioicol = fiolcol + 1;
            }
         }

         /***********************************************************************/
         /*                                                                     */
         /* Step 4: If FORTRAN 66 type alphabetic information is being read     */
         /* into the variable, accept it and loop back to step 1. If you wish   */
         /* to impose strict FORTRAN 77 type I/O then comment out the following */
         /* statement.                                                          */
         /*                                                                     */
         /***********************************************************************/

         else if(fiofspec == 12) fioralph((char*)value,sizeof(longlong),fioiwd);

         /***********************************************************************/
         /*                                                                     */
         /* Step 5: If an "I" format specification is encountered, make certain */
         /* that a fixed field is there and do the conversion. Upon completion, */
         /* loop back to step 1.                                                */
         /*                                                                     */
         /***********************************************************************/

         else if(fiofspec == 14)
         {
            if(fioiwd == 0) fioiwd = 16;
            fiorchk(fioiwd);
            fiondec = 0;
            lv = (longlong)(fiostod(fiocrec+fioicol,fioiwd));
            *value = lv;
            if(fioerc) fioicol += fioerc;
            else fioicol += fioiwd;
         }

         /***********************************************************************/
         /*                                                                     */
         /* Step 6: A "Q" format specification has been entered which requests  */
         /* the length of the remaining input record.                           */
         /*                                                                     */
         /***********************************************************************/

         else if(fiofspec == 20)
         {
            lv = fiolcol - fioicol + 1;
            *value = lv;
         }

         /***********************************************************************/
         /*                                                                     */
         /* Step 7: An invalid specification has been encountered. Set the      */
         /* error flag accordingly and loop back to step 1.                     */
         /*                                                                     */
         /***********************************************************************/

         else fioier = ERDL_IFS;
      }

      /**************************************************************************/
      /*                                                                        */
      /* Step 8: Either all values have been read or an error has occurred.     */
      /* Check for an error and if one is present perform the user specified    */
      /* action.                                                                */
      /*                                                                        */
      /**************************************************************************/

   fiordl9:
      return fioerror(0);
   }
   /*===========================================================================*/
   /*                                                                           */
   /* fiords: Read FORTRAN String                                               */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* Reads a sequence of fixed length strings, stored one after another, from  */
   /* the current input file in accordance with the current format              */
   /* specification. Each string is assumed to have its own corresponding       */
   /* format specification if a formatted read is being performed.              */
   /*                                                                           */
   /* Argument(s)  Description of use                                           */
   /* -----------  ------------------                                           */
   /* str          Points to start of strings                                   */
   /* nstring,     Length of each string                                        */
   /* nval         Number of strings to be read                                 */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if there is no error flag set, else an error code. See the         */
   /* general discussion if FORTRAN I/O capabilities for a listing of           */
   /* the possible error codes.                                                 */
   /*                                                                           */
   /*===========================================================================*/

#ifdef FPROTOTYPE
   int fiords(char* str,int nstring,int nval)
#else
      int fiords(str,nstring,nval)
      char* str;                        /* Points to start of strings */
   int nstring;                      /* Length of each string */
   int nval;                         /* Number of strings to be read */
#endif
   {
      int i;                    /* Dummy counter through strings */
      int nc;                   /* Number of characters in field */

      /*------------------------------------------------------------------------*/
      /*                                                                        */
      /* Step 1: Move through the strings one at a time until all have been     */
      /* read or until an error is encountered. Note that if the error flag is  */
      /* already set when this function is called, then it will exit            */
      /* immediately.                                                           */
      /*                                                                        */
      /*------------------------------------------------------------------------*/

      for(i = 0; (i < nval) && !fioier; i++,str+=nstring)
      {

         /*---------------------------------------------------------------------*/
         /*                                                                     */
         /* Step 2: Obtain the format specifications until a variable related   */
         /* one is encountered.                                                 */
         /*                                                                     */
         /*---------------------------------------------------------------------*/

         fiofinp(0);
         if(fioier) goto fiords9;

         /*---------------------------------------------------------------------*/
         /*                                                                     */
         /* Step 3: If free-field input is being performed, read the next       */
         /* string from the input stream using free-form conventions. Then loop */
         /* back to step 1.                                                     */
         /*                                                                     */
         /*---------------------------------------------------------------------*/

         if(fiofspec == 0)
         {
            if(fioffld())
            {
               /*
                 FKGNOTE: prior to 8.50 this was simply "fioraplh(str,nstring,nstring). I 
                 changed it to look for a terminating comma when reading free-characters
                 into an alpha variable to conform to the Lahey convention. This may require
                 a flag at some point. Note for Lahey I also extended the logic to look
                 for both single and double quotes (fioralph changed also for this).
               */
               if(fiocrec[fioicol] == '\'' || fiocrec[fioicol] == '"') nc = 0;
               else
               {
                  for(nc = fioicol; nc <= fiolcol; nc++)
                  {
                     if(fiocrec[nc] == ',') break;
                  }
                  nc -= fioicol;
               }
               if(nc == 0) nc = nstring;
               fioralph(str,nstring,nc);
            }
         }

         /*---------------------------------------------------------------------*/
         /*                                                                     */
         /* Step 4: If an "A" format specification is encountered, determine    */
         /* the field width and read the string. Note that a "A" with no width  */
         /* specified, then it is assumed to be equivalent to a "An"            */
         /* specification, where "n" is the string length. Then loop back to    */
         /* step 1.                                                             */
         /*                                                                     */
         /*---------------------------------------------------------------------*/

         else if(fiofspec == 12)
         {
            if(fioiwd) nc = fioiwd;
            else nc = nstring;
            fioralph(str,nstring,nc);
         }

         /*---------------------------------------------------------------------*/
         /*                                                                     */
         /* Step 5: An invalid format specification has been found. Set the     */
         /* error code accordingly and loop back to step 1.                     */
         /*                                                                     */
         /*---------------------------------------------------------------------*/

         else fioier = ERDS_IFS;
      }

      /*------------------------------------------------------------------------*/
      /*                                                                        */
      /* Step 6: Either all strings have been read or an error has occurred.    */
      /* Check for an error and if one is present perform the user specified    */
      /* action.                                                                */
      /*                                                                        */
      /*------------------------------------------------------------------------*/

   fiords9:
      return fioerror(0);
   }
   /*===========================================================================*/
   /*                                                                           */
   /* fiordt: Read FORTRAN Truth-Value Vector                                   */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* Reads a vector of truth-values (long logical values) from the current     */
   /* input file in accordance with the current format specification. Note      */
   /* that in this function each individual truth-value is assumed to have      */
   /* its own corresponding format specification if a formatted read is being   */
   /* performed.                                                                */
   /*                                                                           */
   /* Argument(s)  Description of use                                           */
   /* -----------  ------------------                                           */
   /* bool         The values to be read                                        */
   /* nval         The number of values to be read                              */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if there is no error flag set, else an error code. See the general */
   /* discussion if FORTRAN I/O capabilities for a listing of the possible      */
   /* error codes.                                                              */
   /*                                                                           */
   /*===========================================================================*/

#ifdef FPROTOTYPE
   int fiordt(ULONG* mybool,int nval)
#else
      int fiordt(mybool,nval)
      ULONG* mybool;                      /* The values to be read */
   int nval;                         /* The number of values to be read */
#endif
   {
      int i;                    /* Dummy counter through values */
      int nc;                   /* Number of characters in field */
      ULONG uv;                 /* Logical value read */

      /*------------------------------------------------------------------------*/
      /*                                                                        */
      /* Step 1: Move through the truth-values one at a time until all have     */
      /* been read or until an error is encountered. Note that if the error     */
      /* flag is already set when this function is called, then it will exit    */
      /* immediately.                                                           */
      /*                                                                        */
      /*------------------------------------------------------------------------*/

      for(i = 0; (i < nval) && !fioier; i++, mybool++)
      {

         /*---------------------------------------------------------------------*/
         /*                                                                     */
         /* Step 2: Obtain the format specifications until a variable related   */
         /* one is encountered.                                                 */
         /*                                                                     */
         /*---------------------------------------------------------------------*/

         fiofinp(0);
         if(fioier) goto fiordt9;

         /*---------------------------------------------------------------------*/
         /*                                                                     */
         /* Step 3: If FORTRAN 66 type alphabetic information is being read     */
         /* into the variable, accept it and loop back to step 1. If you wish   */
         /* to inpose strict FORTRAN 77 type I/O then comment out the following */
         /* statement and the "else" logic following it.                        */
         /*                                                                     */
         /*---------------------------------------------------------------------*/

         if(fiofspec == 12) fioralph((char*)mybool,sizeof(LONG),fioiwd);
         else
         {

            /*------------------------------------------------------------------*/
            /*                                                                  */
            /* Step 4: If free-field input is being performed, read the next    */
            /* character from the input stream using free-form conventions. If  */
            /* it is a 'T' or 't' then  set the value to true, else set it to   */
            /* FALSE. Then loop back to step 1.                                 */
            /*                                                                  */
            /*------------------------------------------------------------------*/

            if(fiofspec == 0)
            {
               if(fioffld())
               {
                  if(fiocrec[fioicol] == '.') fioicol++;
                  if(fiocrec[fioicol] == 't' || fiocrec[fioicol] == 'T')
                     uv = 1;
                  else uv = 0;
#ifdef ISUNALIGNED
                  put32u((char*)(mybool),uv);
#else
                  *mybool = uv;
#endif
                  do
                  {
                     i = fiocrec[++fioicol];
                  } while(i != ',' && i != ' ' && i != '\0' && i != '/');
               }
            }

            /*------------------------------------------------------------------*/
            /*                                                                  */
            /* Step 5: An invalid format specification has been found. Set the  */
            /* error code accordingly and loop back to step 1.                  */
            /*                                                                  */
            /*------------------------------------------------------------------*/

            else if(fiofspec != 13) fioier = ERDT_IFS;

            /*------------------------------------------------------------------*/
            /*                                                                  */
            /* Step 6: If an "L" format specification is encountered, determine */
            /* the field width and read the first nonblank character. If it is  */
            /* a 'T' or 't' then set the value to true, else set it to FALSE.   */
            /* Then loop back to step 1.                                        */
            /*                                                                  */
            /*------------------------------------------------------------------*/

            else
            {
               if(fioiwd == 0) fioiwd = 16;
               fiorchk(fioiwd);
               nc = fioicol;
               fioicol += fioiwd;
               while(nc < fioicol && fiocrec[nc] == ' ') nc++;
               if(nc < (fioicol-1) && fiocrec[nc] == '.') nc++;
               if(nc < fioicol && (fiocrec[nc] == 't' || fiocrec[nc] == 'T'))
                  uv = 1;
               else uv = 0;
#ifdef ISUNALIGNED
               put32u((char*)(mybool),uv);
#else
               *mybool = uv;
#endif
            }
         }
      }

      /*------------------------------------------------------------------------*/
      /*                                                                        */
      /* Step 7: Either all truth-values have been read or an error has         */
      /* occurred. Check for an error and if one is present perform the user    */
      /* specified action.                                                      */
      /*                                                                        */
      /*------------------------------------------------------------------------*/
   fiordt9:
      return fioerror(0);
   }
   /*===========================================================================*/
   /*                                                                           */
   /* fiordu: Read FORTRAN Unsigned Char Vector                                 */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* Reads a vector of character values from the current input file in         */
   /* accordance with the current format specification. In this implementation  */
   /* unsigned char is derived from the nonstandard FORTRAN type LOGICAL*1.     */
   /* Therefore, L formatting conventions are assumed. Note that in this        */
   /* function each individual character is assumed to have its own correspond- */
   /* ing format specification if a formatted read is being performed.          */
   /*                                                                           */
   /* Argument(s)  Description of use                                           */
   /* -----------  ------------------                                           */
   /* c            Points to characters to be read                              */
   /* nval         Number of characters to read                                 */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if there is no error flag set, else an error code. See the general */
   /* discussion if FORTRAN I/O capabilities for a listing of the possible      */
   /* error codes.                                                              */
   /*                                                                           */
   /*===========================================================================*/

#ifdef FPROTOTYPE
   int fiordu(unsigned char* c,int nval)
#else
      int fiordu(c,nval)
      unsigned char* c;                 /* Points to characters to be read */
   int nval;                         /* Number of characters to read */
#endif
   {
      int i;                    /* Dummy counter through values */
      unsigned short ival;      /* Dummy short integer value */

      for(i = 0; i < nval; i++, c++)
      {
         ival = *c;
         fiordb(&ival,1);
         *c = (unsigned char) ival;
      }
      return fioerror(0);
   }
   /*===========================================================================*/
   /*                                                                           */
   /* fiordx: Read FORTRAN Complex Values                                       */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* Reads a vector of single precision complex values from the current input  */
   /* file in accordance with the current format specification. Each value is   */
   /* assumed to have its own corresponding format specification if a formatted */
   /* read is being performed.                                                  */
   /*                                                                           */
   /* Argument(s)  Description of use                                           */
   /* -----------  ------------------                                           */
   /* value        Points to values to be read                                  */
   /* nval         Number of values to be read                                  */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if there is no error flag set, else an error code. See the general */
   /* discussion if FORTRAN I/O capabilities for a listing of the possible      */
   /* error codes.                                                              */
   /*                                                                           */
   /*===========================================================================*/

#ifdef FPROTOTYPE
   int fiordx(complex* value,int nval)
#else
      int fiordx(value,nval)
      complex* value;                   /* Points to values to be read */
   int nval;                         /* Number of values to be read */
#endif
   {
      int i;
      float temp;

      /*---------------------------------------------------------------------------*/
      /*                                                                           */
      /* If this is a list directed read, then move through the values one at a    */
      /* time until all have been read or until an error is encountered. Note that */
      /* if the error flag is already set when this function is called, then it    */
      /* will exit immediately. If this is a formatted read then use the floating- */
      /* point read routine to read the values.                                    */
      /*                                                                           */
      /*---------------------------------------------------------------------------*/

      if(fioier) goto fiordx9;
      if(!fionfmt)
      {
         fiofspec = fiondec = fioiwd = fioblkn = 0;
         for(i = 0; i < nval; i++,value++)
         {
            if(fioffld())
            {
               if(fioier) goto fiordx9;
               if(fiocrec[fioicol] != '(')
               {
                  fioier = ERDX_MLP;
                  goto fiordx9;
               }
               fioicol++;
               fiospace();
               if(fioier) goto fiordx9;
               temp = (float)(fiostod(fiocrec+fioicol,fiolcol-fioicol+1));
               if(fioier) goto fiordx9;
#ifdef ISUNALIGNED
               putflt((char*)(value),temp);
#else
               value->cr = temp;
#endif
               if(fioerc) fioicol += (fioerc-1);
               else fioicol = fiolcol + 1;
               fiospace();
               if(fioier) goto fiordx9;
               if(fiocrec[fioicol] != ',')
               {
                  fioier = ERDX_COM;
                  goto fiordx9;
               }
               fioicol++;
               fiospace();
               if(fioier) goto fiordx9;
               temp = (float)(fiostod(fiocrec+fioicol,fiolcol-fioicol+1));
               if(fioier) goto fiordx9;
#ifdef ISUNALIGNED
               putflt((char*)(value)+sizeof(float),temp);
#else
               value->ci = temp;
#endif
               if(fioerc) fioicol += (fioerc-1);
               else fioicol = fiolcol + 1;
               fiospace();
               if(fioier) goto fiordx9;
               if(fiocrec[fioicol] != ')')
               {
                  fioier = ERDX_MRP;
                  goto fiordx9;
               }
               fioicol++;
            }
         }
      }
      else fiordf((float*)value,nval*2);
   fiordx9:
      return fioerror(0);
   }
   /*===========================================================================*/
   /*                                                                           */
   /* fiordz: Read FORTRAN Double Complex Values                                */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* Reads a vector of double precision complex values from the current input  */
   /* file in accordance with the current format specification. Each value is   */
   /* assumed to have its own corresponding format specification if a formatted */
   /* read is being performed.                                                  */
   /*                                                                           */
   /* Argument(s)  Description of use                                           */
   /* -----------  ------------------                                           */
   /* value        Points to values to be read                                  */
   /* nval         Number of values to be read                                  */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if there is no error flag set, else an error code. See the general */
   /* discussion if FORTRAN I/O capabilities for a listing of the possible      */
   /* error codes.                                                              */
   /*                                                                           */
   /*===========================================================================*/

#ifdef FPROTOTYPE
   int fiordz(dcomplex* value,int nval)
#else
      int fiordz(value,nval)
      dcomplex* value;                  /* Points to values to be read */
   int nval;                         /* Number of values to be read */
#endif
   {
      int i;
      double temp;

      /*---------------------------------------------------------------------------*/
      /*                                                                           */
      /* If this is a list directed read, then move through the values one at a    */
      /* time until all have been read or until an error is encountered. Note that */
      /* if the error flag is already set when this function is called, then it    */
      /* will exit immediately. If this is a formatted read then use the floating- */
      /* point read routine to read the values.                                    */
      /*                                                                           */
      /*---------------------------------------------------------------------------*/

      if(fioier) goto fiordz9;
      if(!fionfmt)
      {
         fiofspec = fiondec = fioiwd = fioblkn = 0;
         for(i = 0; i < nval; i++,value++)
         {
            if(fioffld())
            {
               if(fioier) goto fiordz9;
               if(fiocrec[fioicol] != '(')
               {
                  fioier = ERDZ_MLP;
                  goto fiordz9;
               }
               fioicol++;
               fiospace();
               if(fioier) goto fiordz9;
               temp = fiostod(fiocrec+fioicol,fiolcol-fioicol+1);
               if(fioier) goto fiordz9;
#ifdef ISUNALIGNED
               putdbl((char*)(value),temp);
#else
               value->dr = temp;
#endif
               if(fioerc) fioicol += (fioerc-1);
               else fioicol = fiolcol + 1;
               fiospace();
               if(fioier) goto fiordz9;
               if(fiocrec[fioicol] != ',')
               {
                  fioier = ERDZ_COM;
                  goto fiordz9;
               }
               fioicol++;
               fiospace();
               if(fioier) goto fiordz9;
               temp = fiostod(fiocrec+fioicol,fiolcol-fioicol+1);
               if(fioier) goto fiordz9;
#ifdef ISUNALIGNED
               putdbl((char*)(value)+sizeof(double),temp);
#else
               value->di = temp;
#endif
               if(fioerc) fioicol += (fioerc-1);
               else fioicol = fiolcol + 1;
               fiospace();
               if(fioier) goto fiordz9;
               if(fiocrec[fioicol] != ')')
               {
                  fioier = ERDZ_MRP;
                  goto fiordz9;
               }
               fioicol++;
            }
         }
      }
      else fiordd((double*)value,nval*2);
   fiordz9:
      return fioerror(0);
   }
   /*===========================================================================*/
   /*                                                                           */
   /* fiorln: Read FORTRAN End-of-Line                                          */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* Completes the current read operation by flushing the current format       */
   /* statement and by setting the current record controls to the end of        */
   /* the current record.                                                       */
   /*                                                                           */
   /* Argument(s): None                                                         */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if there is no error flag set, else an error code. See the         */
   /* general discussion if FORTRAN I/O capabilities for a listing of           */
   /* the possible error codes.                                                 */
   /*                                                                           */
   /*===========================================================================*/

#ifdef FPROTOTYPE
   int fiorln(void)
#else
      int fiorln()
#endif
      {
         LONG ioiad;

         /*---------------------------------------------------------------------------*/
         /*                                                                           */
         /* Perform any additional nonvariable related input format operations, clear */
         /* the current format controls, set the next input position beyond the end   */
         /* of the record, and do any needed error processing.                        */
         /*                                                                           */
         /*---------------------------------------------------------------------------*/

         if(!fioier)
         {
            if(!(fiocurf->fstat & FBINARY))
            {
               fiofinp(1);
               fiofend();
               fioicol = fiolcol + 1;
            }
            else if(!(fiocurf->fstat & FDIRECT))
            {
               if(!(fiocurf->fstat & FINTERNAL))
               {
                  ioiad = posbinf(*(binfile*)&(fiocurf->ffile));
                  ioiad += (fiocurf->frsize + sizeof(LONG));
                  putbinf(*(binfile*)&(fiocurf->ffile),ioiad);
               }
               else
               {
                  ioiad = fiocurf->frecrd + fiocurf->frsize;
                  while(ioiad % sizeof(LONG)) ioiad++;
                  fiocurf->frecrd = ioiad + sizeof(LONG);
               }
            }
         }
         else fionfmt = 0;
         return fioerror(0);
      }
   /*===========================================================================*/
   /*                                                                           */
   /* ftnread: FORTRAN Read Statement                                           */
   /*                                                                           */
   /* Purpose: Executes the FORTRAN READ statement when translated via a non-   */
   /* optimized user bias. In addition to the logical unit number,this function */
   /* takes a variable number of parameters which specify the actual data to be */
   /* supplied to the detailed read functions. The parameter type codes passed  */
   /* to this function are as follows:                                          */
   /*                                                                           */
   /* Code   Parameter  Description of Use                                      */
   /* ----   ---------  ------------------                                      */
   /*  1     LONG*      Points to an error return variable                      */
   /*  2     ---        Indicates that list-directed I/O is being performed     */
   /*  3     char**,int Points to a full FORMAT specification                   */
   /*  4     char*      Points to a FORMAT string                               */
   /*  5     int        Specifies a record number                               */
   /*  9     ---        Specifies that more operations are to be performed      */
   /* 10     int        Specifies that a series of operations are desired       */
   /* 11                Introduces a short integer value                        */
   /* 12                Introduces a double precision value                     */
   /* 13                Introduces a short logical value                        */
   /* 14                Introduces a char value                                 */
   /* 15                Introduces a long value                                 */
   /* 16                Introduces a float value                                */
   /* 17                Introduces a long logical value                         */
   /* 18                Introduces an unsigned char value                       */
   /* 19                Introduces a complex value                              */
   /* 20                Introduces a string value                               */
   /* 21                Introduces a C-string value                             */
   /*                                                                           */
   /* Return value: A zero if the open went well, else an error code. See the   */
   /* general discussion of FORTRAN I/O capabilities for a listing of the       */
   /* possible error codes.                                                     */
   /*                                                                           */
   /*===========================================================================*/

#ifdef VARUNIXV
   int ftnread(va_alist) va_dcl      /* FORTRAN Read Statement */
#else
      int ftnread(int lun,...)
#endif
      {
         va_list argptr;
         int oper;
         int len;
         int ier;                  /* Error processing type code */
         char* str;                /* Dummy pointer */
         char** fmt;               /* Format string pointer */
         int ilun;                 /* If 0,file struc not been established */
         LONG* iostat;             /* Error return status code */
         int sval;                 /* Dummy short integer value */
         int ndo;                  /* Implied doloop count */
         char* ifile;              /* Pointer to internal file storage */
         int nifile;               /* Internal file record length */
         short* ShortPtr;          /* Short value pointer */
         double* DoublePtr;        /* Double precision value pointer */
         unsigned short* BooleanPtr;/* Boolean value pointer */
         char* CharacterPtr;       /* Character value pointer */
         LONG* LongPtr;            /* Long value pointer */
         float* FloatPtr;          /* Single precision value pointer */
         ULONG* LogicalPtr;        /* Logical value pointer */
         unsigned char* BytePtr;   /* Byte value pointer */
         complex* ComplexPtr;      /* Complex value pointer */
         dcomplex* DcomplexPtr;    /* Double complex value pointer */
         char* StringPtr;          /* String value pointer */
         longlong* LongLongPtr;

         /*------------------------------------------------------------------------*/
         /*                                                                        */
         /* Step 1: Initialize the file structure creation variable in accordance  */
         /* with whether this is a MORE call or an intial call, and intialize the  */
         /* machinery needed to deal with the variable number of arguments.        */
         /*                                                                        */
         /*------------------------------------------------------------------------*/

#ifdef VARUNIXV
         int lun;

         va_start(argptr);
         lun = va_arg(argptr,int);
#else
         va_start(argptr,lun);
#endif
         if(lun == REREAD) {
            lun = fiocurf->funit;
            fiocurf->fstat1 |= FREREAD;
         }
         ilun = mode;
         nifile = len = 0;
         ifile = NULL;
         fiostat = NULL;
         if(lun == NAMED) {
            ifile = va_arg(argptr,char*);
            nifile = va_arg(argptr,int);
            fioname(ifile,nifile,0);
            if(fioier == 0) lun = fiocurf->funit;
         }
         /*------------------------------------------------------------------------*/
         /*                                                                        */
         /* Step 2: If this is an intialization call obtain the first parameter    */
         /* from the argument list and if it is an error status specification      */
         /* establish it accordingly, then obtain the next parameter from the list.*/
         /* Else if this is a MORE call, the first parameter code is contained in  */
         /* the "lun" parameter.                                                   */
         /*                                                                        */
         /*------------------------------------------------------------------------*/

         if(!mode)
         {
            fioier = fioerch = 0;
            iomode = 0;
            if(lun == INTERNAL) {       /* This is an internal file */
               ifile = va_arg(argptr,char*);
               nifile = va_arg(argptr,int);
            }
            oper = va_arg(argptr,int);
            if(oper == 1) {
               iostat = va_arg(argptr,LONG*);
               ier = va_arg(argptr,int);
               fiostatus(iostat,ier);
               if(fioier != 0) {
                  va_end(argptr);
                  return fioerror(1);
               }
               oper = va_arg(argptr,int);
            }
         }
         else oper = lun;

         /*------------------------------------------------------------------------*/
         /*                                                                        */
         /* Step 3: A parameter type code has been obtained from the parameter     */
         /* list. If it is not part of the iolist, process it and obtain and       */
         /* additional parameters until an iolist parameter is found.              */
         /*                                                                        */
         /*------------------------------------------------------------------------*/

      ftnread3:
         if(oper > 8 || oper == 0) {
            ndo = 1;
            if(!ilun) {
               if(!iomode) {
                  if(lun == INTERNAL) fiointu(ifile,nifile,3);
                  else fiolun(lun,3);
               }
               else {
                  if(lun == -9998) fiostio(2);
                  else if(lun == INTERNAL) fiointu(ifile,nifile,2);
                  else fiolun(lun,2);
               }
            }
            goto ftnread4;
         }
         switch(oper) {
         case 2:                        /* List-directed READ is being requested. */
            iomode = 1;
            break;
         case 3:                        /* Full format spec is being introduced */
            iomode = 1;
            fmt = va_arg(argptr,char**);
            len = va_arg(argptr,int);
            fiofini(fmt,len);
            break;
         case 4:                        /* A variable format is being introduced */
            iomode = 1;
            str = va_arg(argptr,char*);
            len = va_arg(argptr,int);
            fiovfini((char**)str,len);
            break;
         case 5:                        /* A particular record has been requested */
            if(lun == INTERNAL) fiointu(ifile,nifile,3);
            else fiolun(lun,3);
            ilun = 1;
            len = va_arg(argptr,int);
            fiorec(len);
            break;
         }
         oper = va_arg(argptr,int);
         goto ftnread3;

         /*------------------------------------------------------------------------*/
         /*                                                                        */
         /* Step 4: An actual value is being read. The information following the   */
         /* parameter and the actual utility to be used depends upon the context.  */
         /* For binary reads there is a pointer of the indicated type followed by  */
         /* the number of elements of that type to be transfered. For coded reads  */
         /* there is a pointer.                                                    */
         /*                                                                        */
         /*------------------------------------------------------------------------*/

      ftnread4:
         if(oper == 0 || oper == 9) {   /* The READ operation has been completed. */
            va_end(argptr);
            if(oper == 9) {
               mode = 1;
               return 0;
            }
            mode = 0;
            fiorln();
            sval = fioerror(1);
            return sval;
         }
         if(oper == 10) {
            ndo = va_arg(argptr,int);
            oper = va_arg(argptr,int);
            goto ftnread4;
         }
         oper -= 11;
         if(iomode != 0) goto ftnread6;
         switch(oper) {
         case 0:                        /* Introduces a short integer value */
            str = (char*)(va_arg(argptr,short*));
            len = sizeof(short);
            break;
         case 1:                        /* Introduces a double precision value */
            str = (char*)(va_arg(argptr,double*));
            len = sizeof(double);
            break;
         case 2:                        /* Introduces a short logical value */
            str = (char*)(va_arg(argptr,unsigned short*));
            len = sizeof(unsigned short);
            break;
         case 3:                        /* Introduces a char value */
            str = va_arg(argptr,char*);
            len = sizeof(char);
            break;
         case 4:                        /* Introduces a long value */
            str = (char*)(va_arg(argptr,LONG*));
            len = sizeof(LONG);
            break;
         case 5:                        /* Introduces a float value */
            str = (char*)(va_arg(argptr,float*));
            len = sizeof(float);
            break;
         case 6:                        /* Introduces a long logical value */
            str = (char*)(va_arg(argptr,ULONG*));
            len = sizeof(ULONG);
            break;
         case 7:                        /* Introduces an unsigned char value */
            str = (char*)(va_arg(argptr,unsigned char*));
            len = sizeof(unsigned char);
            break;
         case 8:                        /* Introduces a complex value */
            str = (char*)(va_arg(argptr,complex*));
            len = sizeof(complex);
            break;
         case 9:                        /* Introduces a string value */
            str = va_arg(argptr,char*);
            len = va_arg(argptr,int);
            break;
         case 10:                       /* Introduces a C-string value */
            str = va_arg(argptr,char*);
            len = 1;
            break;
         case 12:
            str = (char*)(va_arg(argptr,longlong*));
            len = sizeof(longlong);
            break;
         default:
         case 11:                       /* Introduces a double complex value */
            str = (char*)(va_arg(argptr,dcomplex*));
            len = sizeof(dcomplex);
            break;
         }
         sval = va_arg(argptr,int);
         fiorbiv(str,len*sval);
         oper = va_arg(argptr,int);
         goto ftnread4;
      ftnread6:
         switch(oper) {
         case 0:                        /* Introduces a short integer value */
            ShortPtr = va_arg(argptr,short*);
            fiordi(ShortPtr,ndo);
            break;
         case 1:                        /* Introduces a double precision value */
            DoublePtr = va_arg(argptr,double*);
            fiordd(DoublePtr,ndo);
            break;
         case 2:                        /* Introduces a short logical value */
            BooleanPtr = va_arg(argptr,unsigned short*);
            fiordb(BooleanPtr,ndo);
            break;
         case 3:                        /* Introduces a char value */
            CharacterPtr = va_arg(argptr,char*);
            fiordc(CharacterPtr,ndo);
            break;
         case 4:                        /* Introduces a long value */
            LongPtr = va_arg(argptr,LONG*);
            fiordl(LongPtr,ndo);
            break;
         case 5:                        /* Introduces a float value */
            FloatPtr = va_arg(argptr,float*);
            fiordf(FloatPtr,ndo);
            break;
         case 6:                        /* Introduces a long logical value */
            LogicalPtr = va_arg(argptr,ULONG*);
            fiordt(LogicalPtr,ndo);
            break;
         case 7:                        /* Introduces an unsigned char value */
            BytePtr = va_arg(argptr,unsigned char*);
            fiordu(BytePtr,ndo);
            break;
         case 8:                        /* Introduces a complex value */
            ComplexPtr = va_arg(argptr,complex*);
            fiordx(ComplexPtr,ndo);
            break;
         case 9:                        /* Introduces a string value */
            StringPtr = va_arg(argptr,char*);
            len = va_arg(argptr,int);
            fiords(StringPtr,len,ndo);
            break;
         case 11:
            DcomplexPtr = va_arg(argptr,dcomplex*);
            fiordz(DcomplexPtr,ndo);
            break;
         case 12:                        /* Introduces a long value */
            LongLongPtr = va_arg(argptr,longlong*);
            fiordll(LongLongPtr,ndo);
            break;
         }
         ndo = 1;
         oper = va_arg(argptr,int);
         goto ftnread4;
      }

   // fiowrit
   /*
     ;/hdr/ ************************************************************************
     ;
     ; Copyright 1986-1998 Promula Development Corporation ALL RIGHTS RESERVED
     ;
     ; name of module: FioWrit.c -- Fortran I/O, Write Operations
     ;
     ; global symbols defined:
     ;
     ; useful notes and assumptions:
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; edit 06/17/98 FKG: Changed names of global symbols to conform to the 
     ;                    Promula version 7.08 API naming conventions. 
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00
     ;
     ;/hdr/ ************************************************************************
   */
#define MATHFUNC                  /* Gives access to mathematical functions */
#define PRIOFUNC                  /* Gives access to PROMULA I/O functions */
#define STRGFUNC                  /* Access to string manipulation functions */
#define RAWMFUNC                  /* Gives access to raw memory functions */
#define VARARGUS                  /* Gives access to variable arguments */
   // #include "platform.h"             /* Define the platform hosting this code */
   // #include "pfclib.h"               /* Define PROMULA.FORTRAN library symbols */
   // #include "chrapi.h"               /* Character processig interface */

#ifdef FPROTOTYPE
   int   fioerror(int clear);
   void  fiofend(void);
   int   fiofini(char** fmt,int nfmt);
   void  fiofout(int context);
   int   fiointu(char* intu,int rsize,int action);
   void  fioltos(LONG value);
   int   fiolun(int lun,int action);
   int   fioname(char* strg,int ns,int status);
   void  fionxtf(void);
   void  fiorec(int irec);
   void  fiostatus(LONG* iostat,int error);
   int   fiostio(int action);
   int   fiovfini(char** fmt,int nfmt);
   char* fiowhexo(int base,char* ival,int vlen);
   void  fiowval(double value);
#else
   extern int   fioerror();          /* Do requested error processing */
   extern void  fiofend();           /* End current format */
   extern int   fiofini();           /* Initialize A FORTRAN format */
   extern void  fiofout();           /* Process output format specification */
   extern int   fiointu();           /* Establish FORTRAN internal unit */
   extern void  fioltos();           /* Convert long to string */
   extern int   fiolun();            /* Establish FORTRAN unit number */
   extern int   fioname();           /* Establish FORTRAN unit by name */
   extern void  fionxtf();           /* Get next format specification */
   extern void  fiorec();            /* Position a FORTRAN file on a record */
   extern void  fiostatus();         /* Set FORTRAN I/O error status */
   extern int   fiostio();           /* Establish FORTRAN standard I/O */
   extern int   fiovfini();          /* Initialize A Variable FORTRAN Format */
   extern char* fiowhexo();          /* Write Hexadecimal or octal constant */
   extern void  fiowval();           /* Write floating point value */
#endif

   extern int      fioconv;          /* General dialect convention flags */
   extern char     fiocrec[FIOBMAX]; /* Coded communications record */
   extern FIOFILE* fiocurf;          /* Pointer to current FORTRAN file */
   extern int      fioefw;           /* Minimum display width for octal or hex */
   extern int      fioerch;          /* Specifies whether error checking present */
   extern int      fioffw;           /* Record width for free-form output */
   extern int      fiofspec;         /* Current format specification */
   extern int      fioier;           /* Code for actual error encountered */
   extern int      fioiwd;           /* Display width */
   extern int      fiomaxc;          /* Maximum characters in output line */
   extern int      fionchar;         /* Number of output characters */
   extern int      fiondec;          /* Display decimal specification */
   extern int      fionfmt;          /* Number of entries in format list */
   extern int      fioplus;          /* Plus sign character */
   extern char*    fiosadr;          /* Format string location */
   extern int      fioscal;          /* Floating point scale factor */
   extern LONG*    fiostat;          /* Returns an error code or zero */

   // static int mode = 0;              /* 1 if waiting for MORE else 0 */
   // static int iomode = 0;            /* 0 if binary write, else 1 */

   /*****************************************************************************/
   /*                                                                           */
   /* FIOWTXT: Write Text Record                                                */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* Writes the current text record to the current file followed by a newline. */
   /* Then it sets the length of the current text record to zero.               */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* None, the function is void.                                               */
   /*                                                                           */
   /* See also:                                                                 */
   /*                                                                           */
   /* Referenced by:                                                            */
   /*                                                                           */
   /* Element       Description of use                                          */
   /* -------       ------------------                                          */
   /* fiowln        Flush currrrent text record upon completion of the current  */
   /*               write sequence.                                             */
   /* fiofout       Execute a "/" output format specification.                  */
   /*                                                                           */
   /*****************************************************************************/

#ifdef FPROTOTYPE
   void fiowtxt(void)
#else
      void fiowtxt()
#endif
      {
         char* s1;                 /* Pointer to internal file storage */
         int nc;                   /* Internal file record size */
         int crlf;                 /* Write end-of-line indicator */
         char* intu;
         int ipos;
         int lpos;
         LONG ioiad;

         /*------------------------------------------------------------------------*/
         /*                                                                        */
         /* Step 1: Make certain that all characters in the record will be written.*/
         /*                                                                        */
         /*------------------------------------------------------------------------*/

         if(fionchar < fiomaxc) fionchar = fiomaxc;

         /*------------------------------------------------------------------------*/
         /*                                                                        */
         /* Step 2: If this is an internal file of some sort, then branch to step  */
         /*                                                                        */
         /*------------------------------------------------------------------------*/

         if(!(fiocurf->fstat & FINTERNAL))
         {

            /*---------------------------------------------------------------------*/
            /*                                                                     */
            /* Step 3: If we have been reading from this non direct access file,   */
            /* and we are now going to be writing to it, then close the file and   */
            /* reinitialize it.                                                    */
            /*                                                                     */
            /*---------------------------------------------------------------------*/

            if((fiocurf->fstat & FEXIST) && !(fiocurf->fstat & FDIRECT))
            {
               fiocurf->fstat ^= FEXIST;
               clstxtf(*(txtfile*)&(fiocurf->ffile));
               *(txtfile*)&(fiocurf->ffile) = initxtf(fiocurf->fname);
            }

            /*---------------------------------------------------------------------*/
            /*                                                                     */
            /* Step 4:                                                             */
            /*                                                                     */
            /*---------------------------------------------------------------------*/

            if(fiocurf->fstat & FDIRECT)
            {
               if(fiocurf->fstat1 & FREADLAST)
               {
                  ioiad = postxtf(*(txtfile*)&(fiocurf->ffile));
                  puttxtf(*(txtfile*)&(fiocurf->ffile),ioiad);
                  fiocurf->fstat1 ^= FREADLAST;
               }
               if(fionchar > fiocurf->frsize) fionchar = (int)(fiocurf->frsize);
               else while(fionchar < fiocurf->frsize) fiocrec[fionchar++] = ' ';
            }

            /*---------------------------------------------------------------------*/
            /*                                                                     */
            /* Step 6:                                                             */
            /*                                                                     */
            /*---------------------------------------------------------------------*/

            else if(fiocurf->fstat & FENDFIL)
            {
               fioier = EWTX_EOF;
               fioerror(0);
               goto fiowtxt9;
            }

            /*---------------------------------------------------------------------*/
            /*                                                                     */
            /* Step 7: We are ready to write the current record to the file. If a  */
            /* newline is required prior to that write, then issue it now.         */
            /*                                                                     */
            /*---------------------------------------------------------------------*/

            if(fiocurf->fstat1 & FNEEDCRLF)
            {
               fiocurf->fstat1 ^= FNEEDCRLF;
               wrteol(*(txtfile*)&(fiocurf->ffile));
               if(!(fiocurf->fstat & FDIRECT)) fiocurf->frsize += 1; /* FKG */
            }

            /*-----------------------------------------------------------*/
            /*                                                                     */
            /* Step 8: Initialize the number of line feeds to be issued with the   */
            /* current record.                                                     */
            /*                                                                     */
            /*---------------------------------------------------------------------*/

            if(fiocurf->fstat1 & FNOCRLF)
            {
               fiocurf->fstat1 ^= FNOCRLF;
               crlf = 0;
            }
            else crlf = 1;
            if(fionchar)
            {
               if(!(fiocurf->fstat & FCARRIG))
               {
                  wrtxtf(*(txtfile*)&(fiocurf->ffile),fiocrec,fionchar);
               }
               else
               {
                  if(fioconv & ZDFLEXF)
                  {
                     if(fiocrec[0] == '#')
                     {
                        fiocrec[0] = 14;
                        wrtxtf(*(txtfile*)&(fiocurf->ffile),fiocrec,fionchar);
                        goto fiowtxt8;
                     }
                  }
                  if(fiocrec[0] == '+' || fiocrec[0] == '$') crlf = 0;
                  else if(fiocrec[0] == '1') wrtffd(*(txtfile*)&(fiocurf->ffile));
                  else if(fiocrec[0] == '0') crlf = 2;
                  if(crlf == 2 && (fioconv & ZDSKPRN) != 0)
                  {
                     wrteol(*(txtfile*)&(fiocurf->ffile));
                     if(!(fiocurf->fstat & FDIRECT)) fiocurf->frsize += 1;/*FKG*/
                     crlf--;
                  }
                  if(fionchar > 1)
                  { 
                     wrtxtf(*(txtfile*)&(fiocurf->ffile),fiocrec+1,fionchar-1);
                  }
               }
            }
         fiowtxt8:
            if(crlf)
            {
               if(crlf == 2)
               {
                  wrteol(*(txtfile*)&(fiocurf->ffile));
                  if(!(fiocurf->fstat & FDIRECT)) fiocurf->frsize += 1; /*FKG*/
               }
               if((fioconv & ZDSKPRN) == 0)
               {
                  wrteol(*(txtfile*)&(fiocurf->ffile));
                  if(!(fiocurf->fstat & FDIRECT)) fiocurf->frsize += 1;/*FKG*/
               }
               else fiocurf->fstat1 |= FNEEDCRLF;
            }
            if(fiocurf->fstat1 & FTERMINAL) fflush(*(txtfile*)&(fiocurf->ffile));
         }
         else if(fiocurf->funit != INTERNAL && fiocurf->fassob == NULL)
         {
            s1 = *(char**)&(fiocurf->ffile);
            nc = (int)(fiocurf->frsize);
            filmem(s1,nc,' ');
            if(fionchar > nc) fionchar = nc;
            if(fionchar) cpymem(fiocrec,s1,fionchar);
            s1 += nc;
            *(char**)&(fiocurf->ffile) = s1;
         }
         else
         {
            if((intu = fiocurf->fassob) == NULL) intu = *(char**)&(fiocurf->ffile);
            ipos = (int) (fiocurf->frecrd);
            lpos = (int) (fiocurf->fmaxrc);
            intu += ipos;
            if((fiocurf->fstat & FEXIST) && !(fiocurf->fstat & FDIRECT))
            {
               fiocurf->fstat ^= FEXIST;
               fiocurf->frecrd = 0;
               intu = *(char**)&(fiocurf->ffile);
               ipos = 0;
            }
            if(fiocurf->fstat & FDIRECT)
            {
               if(fionchar > fiocurf->frsize) fionchar = (int)(fiocurf->frsize);
               else while(fionchar < fiocurf->frsize) fiocrec[fionchar++] = ' ';
            }
            else if(fiocurf->fstat & FENDFIL) fiocurf->fstat ^= FENDFIL;
            if(fiocurf->fstat1 & FNOCRLF)
            {
               crlf = 0;
               fiocurf->fstat1 ^= FNOCRLF;
            }
            else crlf = 1;
            while(fionchar > 1 && fiocrec[fionchar-1] == ' ') fionchar--;
            if(fionchar)
            {
               if((fionchar + ipos) >= lpos)
               {
                  fioier = EWTX_EOF;
                  fioerror(0);
                  goto fiowtxt9;
               }
               cpymem(fiocrec,intu,fionchar);
               ipos += fionchar;
            }
            if(crlf)
            {
               *(intu+fionchar) = '\n';
               ipos++;
            }
            fiocurf->frecrd = ipos;
         }
      fiowtxt9:
         fionchar = 0;
      }

   /*****************************************************************************/
   /*                                                                           */
   /* FIOFOUT: Formatted Output Operations                                      */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* Performs non-variable related formatted output functions until an end-of- */
   /* format or a rescan or variable related specification is encountered. The  */
   /* actual operations performed by this function are as follows:              */
   /*                                                                           */
   /* Specification  Code Description                                           */
   /* -------------  ---- -----------                                           */
   /* nH               1  Write characters from a holerith string               */
   /* "c1..cn"         1  Write characters from a delimited string              */
   /* nX               2  Skip right n places                                   */
   /* TRn              2  Skip right n places                                   */
   /* Tn               3  Move to position n                                    */
   /* TLn              4  Skip left n places                                    */
   /* SS               5  Set the plus sign to a space                          */
   /* SP               6  Set the plus sign to a +                              */
   /* BN               7  Set blanks to null (no operation for output)          */
   /* BZ               8  Set blanks to zero (no operation for output)          */
   /* /                9  Physically write the current record                   */
   /* nP              10  Set the floating scale factor to n                    */
   /* )               11  End-of-Format write current record                    */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* None the function is void; however, the global variable "fioier" may be   */
   /* set to an error code if a problem is encountered.                         */
   /*                                                                           */
   /* See also:                                                                 */
   /*                                                                           */
   /* Referenced by:                                                            */
   /*                                                                           */
   /* Element       Description of use                                          */
   /* -------       ------------------                                          */
   /* fiowln        Process remaining nonvariable specifications                */
   /* fiowrb        Get specification for a boolean value                       */
   /* fiowrc        Get specification for single character                      */
   /* fiowrd        Get specification for a double value                        */
   /* fiowrf        Get specification for a float value                         */
   /* fiowri        Get specification for a short integer value                 */
   /* fiowrl        Get specification for a long integer value                  */
   /* fiowrs        Get specification for a string                              */
   /* fiowrt        Get specification for a truth-value                         */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* See detailed steps below.                                                 */
   /*                                                                           */
   /*****************************************************************************/

#ifdef FPROTOTYPE
   void fiofout(int context)
#else
      void fiofout(context)
      int context;                   /* Context of call 0 = value, 1 = end */
#endif
   {
      int i;                    /* Dummy skip counter */

      /**************************************************************************/
      /*                                                                        */
      /* Step 1: Obtain the next format specification from the generic format   */
      /* processing function and if an error occurs or if the specification is  */
      /* variable related or a rescan indication then return that specification */
      /* to the calling function for processing. Else branch to perform the     */
      /* indicated operation.                                                   */
      /*                                                                        */
      /**************************************************************************/

   fiofout1:
      fionxtf();
      if(fioier || fiofspec > 11) return;
      switch(fiofspec)
      {
      case -2:
         fiocurf->fstat1 |= FNOCRLF;
         goto fiofout1;
      case -1:
         if(!context) goto fiofout1;
         break;
      case 0:
         if((fionchar == 0) && !(fiocurf->fstat & FINTERNAL)) 
            fiocrec[fionchar++] = ' ';
         break;

         /***********************************************************************/
         /*                                                                     */
         /* Step 2: A "display string" specification is to be executed. Do so,  */
         /* and loop back to step 1.                                            */
         /*                                                                     */
         /***********************************************************************/

      case 1: 
         for(i = 0; i < fioiwd; i++)
         {
            if(*(fiosadr+i) == fiondec) i++;
            fiocrec[fionchar++] = *(fiosadr+i);
         }
         goto fiofout1;

         /***********************************************************************/
         /*                                                                     */
         /* Step 3: A "skip right n spaces" specification is to be executed. Do */
         /* so by inserting the appropriate number of blanks and loop back to   */
         /* step 1.                                                             */
         /*                                                                     */
         /***********************************************************************/
      case 2:
         for(i = 0; i < fioiwd; i++)
         {
            if(fionchar >= fiomaxc) fiocrec[fionchar] = ' ';
            fionchar++;
         }
         goto fiofout1;

         /***********************************************************************/
         /*                                                                     */
         /* Step 4: A "move to position n" specification is to be executed.     */
         /* Reset the number of characters in the current text record if the    */
         /* position is to the left; else insert the needed number of blanks to */
         /* move to the right. Then loop back to step 1. Note that the stated   */
         /* position number must be decremented by one to compensate for the    */
         /* fact the FORTRAN FORMAT assumes that chararacter number 1 is at     */
         /* position 1 and not 0 as in C.                                       */
         /*                                                                     */
         /***********************************************************************/

      case 3:
         fioiwd--;
         if(fionchar > fioiwd)
         {
            fiomaxc = fionchar;
            fionchar = fioiwd;
         }
         else
         {
            while(fionchar < fioiwd)
            {
               if(fionchar >= fiomaxc) fiocrec[fionchar] = ' ';
               fionchar++;
            }
         }
         goto fiofout1;

         /***********************************************************************/
         /*                                                                     */
         /* Step 5: A "skip left n places" specification is to be executed.     */
         /* Decrement the number of characters in the current text record       */
         /* accordingly, making certain that the number of characters does not  */
         /* go negative. Then loop back to step 1.                              */
         /*                                                                     */
         /***********************************************************************/

      case 4: 
         fiomaxc = fionchar;
         i = fioiwd;
         if(i > fionchar) i = fionchar;
         fionchar -= i;
         goto fiofout1;

         /***********************************************************************/
         /*                                                                     */
         /* Step 6: A "set plus sign to space" specification is to be executed. */
         /* Set the appropriate global variables and loop back to step 1.       */
         /*                                                                     */
         /***********************************************************************/

      case 5:
         fioplus = ' ';       
         ChrFlags |= CHR_PLUSSIGN;
         ChrFlags ^= CHR_PLUSSIGN;
         goto fiofout1;

         /***********************************************************************/
         /*                                                                     */
         /* Step 7: A "set plus sign to plus" specification is to be executed.  */
         /* Set the appropriate global variables and loop back to step 1.       */
         /*                                                                     */
         /***********************************************************************/

      case 6:
         fioplus = '+';
         ChrFlags |= CHR_PLUSSIGN;
         goto fiofout1;

         /***********************************************************************/
         /*                                                                     */
         /* Step 8: A "set input blanks to null or zero" specification is to be */
         /* executed. Since this specification has no relevance to output       */
         /* processing it is simply ignored here. One could make this an error  */
         /* by setting "fioier". Then loop back to step 1.                      */
         /*                                                                     */
         /***********************************************************************/

      case 7: case 8:
         goto fiofout1;

         /***********************************************************************/
         /*                                                                     */
         /* Step 9: A "physically write current line" specification is the be   */
         /* executed. Do so and loop back to step 1.                            */
         /*                                                                     */
         /***********************************************************************/

      case 9:
         fiowtxt();
         fiomaxc = -1;
         goto fiofout1;

         /***********************************************************************/
         /*                                                                     */
         /* Step 10: A "set scale factor" specification is to be executed. Set  */
         /* the global variable and loop back to step 1.                        */
         /*                                                                     */
         /***********************************************************************/

      case 10:
         fioscal = fioiwd;
         goto fiofout1;

         /***********************************************************************/
         /*                                                                     */
         /* Step 11: An End-of-format has been encountered. If processing       */
         /* values, physically write the current line and loop back to step 1;  */
         /* else end local processing.                                          */
         /*                                                                     */
         /***********************************************************************/

      case 11:
         if(!context)
         {
            if(fiocurf->fstat1 & FNOCRLF) fiocurf->fstat1 ^= FNOCRLF;
            fiowtxt();
            goto fiofout1;
         }
      }
   }

   /*****************************************************************************/
   /*                                                                           */
   /* FIOWALPH: Write Alphabetic Information                                    */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* Write a character string, left-justified, blank-filled to the right into  */
   /* a fixed width field. If the character string is longer than the field,    */
   /* then only the left-most characters are written. In addition to writing    */
   /* actual characters, this function also displays values using octal or      */
   /* hexidecimal notation.                                                     */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* None, the function is void. The effect of its processing is reflected     */
   /* in the various global format control variables.                           */
   /*                                                                           */
   /* See also:                                                                 */
   /* Process:                                                                  */
   /*                                                                           */
   /* See description of detailed steps below.                                  */
   /*                                                                           */
   /*****************************************************************************/

#ifdef FPROTOTYPE
   void fiowalph(char* alpha,int nalpha,int nfield)
#else
      void fiowalph(alpha,nalpha,nfield)
      char* alpha;                   /* Character string */
   int nalpha;                    /* Length of character string */
   int nfield;                    /* Width of field */
#endif
   {
      int i;
      int c;
      char* xcons;
      int nc;

      /*------------------------------------------------------------------------*/
      /*                                                                        */
      /* Step 1: If a simple character display is desired, enter the characters */
      /* into the output record and end local processing.                       */
      /*                                                                        */
      /*------------------------------------------------------------------------*/

      if(fiondec == 0)
      {
         if(nfield == 0) nfield = nalpha;
         else if(nalpha > nfield) nalpha = nfield;
         while(nalpha < nfield)
         {
            fiocrec[fionchar++] = ' ';
            nfield--;
         }
         nc = fionchar;
         for(i = 0; i < nalpha; i++)
         {
            c = *(alpha+i);
            fiocrec[fionchar++] = (unsigned char) c;
         }
      }

      /*------------------------------------------------------------------------*/
      /*                                                                        */
      /* Step 2: Octal or hexidecimal notation is desired. First determine the  */
      /* number of bits to be displayed and the minimum bit number of each      */
      /* digit value.                                                           */
      /*                                                                        */
      /*------------------------------------------------------------------------*/

      else
      {
         if(fiondec == -1) xcons = fiowhexo(8,alpha,nalpha);
         else xcons = fiowhexo(16,alpha,nalpha);
         nc = strlen(xcons);
         c = fioefw;
         if(c <= 0) c = 1;
         while(nc > c && *xcons == '0')
         {
            xcons++;
            nc--;
         }
         if(nfield == 0) nfield = nc;
         else if(nc > nfield) nc = nfield;
         while(nc < nfield)
         {
            fiocrec[fionchar++] = ' ';
            nfield--;
         }
         for(i = 0; i < nc; i++) fiocrec[fionchar++] = *(xcons+i);
      }
   }

   /*****************************************************************************/
   /*                                                                           */
   /* FIOWLN: Write FORTRAN End-of-Line                                         */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* Completes the current read operation by flushing the current format       */
   /* statement and writing out the current record.                             */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if there is no error flag set, else an error code. See the general */
   /* discussion if FORTRAN I/O capabilities for a listing of the possible      */
   /* error codes.                                                              */
   /*                                                                           */
   /* See also:                                                                 */
   /*                                                                           */
   /* Referenced by:                                                            */
   /*                                                                           */
   /* Element       Description of use                                          */
   /* -------       ------------------                                          */
   /* FORTRAN       WRITE statement                                             */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* Perform any additional nonvariable related output format operations,      */
   /* clear the current format controls, write out any current text record, and */
   /* do any needed error processing. For unformatted sequential files, write   */
   /* the record length at the beginning of the record and at the end of the    */
   /* record.                                                                   */
   /*                                                                           */
   /*****************************************************************************/

#ifdef FPROTOTYPE
   int fiowln(void)
#else
      int fiowln()
#endif
      {
         LONG ioiad;
         LONG length;
         char* intu;
         int ipos;

         if(fioier) goto fiowln9;
         if(fiocurf->fassov != NULL) *(fiocurf->fassov) += 1;
         if(!(fiocurf->fstat & FBINARY))
         {
            fiofout(1);
            fiowtxt();
            fiofend();
         }
         else if(!(fiocurf->fstat & FDIRECT))
         {
            if(!(fiocurf->fstat & FINTERNAL))
            {
               if(fiocurf->frecrd >= 0)
               {
                  length = fiocurf->frecrd;
                  fiocurf->frecrd = -1;
                  wrbinf(*(binfile*)&(fiocurf->ffile),&length,sizeof(LONG));
               }
               else
               {
                  ioiad = posbinf(*(binfile*)&(fiocurf->ffile));
                  if(ioiad > fiocurf->frsize)
                  {
                     length = ioiad - fiocurf->frsize;
                     wrbinf(*(binfile*)&(fiocurf->ffile),&length,sizeof(LONG));
                     putbinf(*(binfile*)&(fiocurf->ffile),
                             fiocurf->frsize-sizeof(LONG));
                     wrbinf(*(binfile*)&(fiocurf->ffile),&length,sizeof(LONG));
                     ioiad += sizeof(LONG);
                     putbinf(*(binfile*)&(fiocurf->ffile),ioiad);
                  }
                  else
                  {
                     length = 0;
                     wrbinf(*(binfile*)&(fiocurf->ffile),&length,sizeof(LONG));
                  }
               }
            }
            else if(fiocurf->fstat & FINTERNAL)
            {
               if((intu = fiocurf->fassob) == NULL) 
                  intu = *(char**)&(fiocurf->ffile);
               ipos = (int) (fiocurf->frecrd);
               while(ipos % sizeof(LONG)) ipos++;
               ioiad = ipos;
               if(ioiad > fiocurf->frsize)
               {
                  length = ioiad - fiocurf->frsize;
                  *(LONG*)(intu+ipos) = length;
                  fiocurf->frecrd = ipos + sizeof(LONG);
                  ipos = (int) (fiocurf->frsize - sizeof(LONG));
                  *(LONG*)(intu+ipos) = length;
               }
               else
               {
                  length = 0;
                  *(LONG*)(intu+ipos) = length;
                  fiocurf->frecrd = ipos + sizeof(LONG);
               }
               *(LONG*)(intu+fiocurf->frecrd) = -1;
            }
         }
      fiowln9:
         return fioerror(0);
      }

   /*****************************************************************************/
   /*                                                                           */
   /* FIOWBIV: FORTRAN Write Binary Values                                      */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* This function write binary values to a file.                              */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if all went well, else an error code. See the general discussion   */
   /* of the FORTRAN I/O capabilities for a listing of the possible error codes.*/
   /*                                                                           */
   /* See also:                                                                 */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* Simply write the values.                                                  */
   /*                                                                           */
   /*****************************************************************************/

#ifdef FPROTOTYPE
   int fiowbiv(VOID* value,int nvalue)
#else
      int fiowbiv(value,nvalue)
      VOID* value;                      /* Points to values being written */
   int nvalue;                       /* Number of bytes to be written */
#endif
   {
      char* intu;
      LONG ipos;
      LONG lpos;

      if(fioier) goto fiowbiv9;
      if((fiocurf->fstat & FEXIST) && !(fiocurf->fstat & FDIRECT))
      {
         if(!(fiocurf->fstat & FINTERNAL))
         {
            fiocurf->fstat ^= FEXIST;
            clsbinf(*(binfile*)&(fiocurf->ffile));
            *(binfile*)&(fiocurf->ffile) = inibinf(fiocurf->fname);
         }
         else fiocurf->frecrd = 0;
      }
      if(!(fiocurf->fstat & FINTERNAL))
      {
         if((fiocurf->fstat & FDIRECT) && (fiocurf->fstat1 & FREADLAST))
         {
            ipos = posbinf(*(binfile*)&(fiocurf->ffile));
            putbinf(*(binfile*)&(fiocurf->ffile),ipos);
            fiocurf->fstat1 ^= FREADLAST;
         }
         if((int)wrbinf(*(binfile*)&(fiocurf->ffile),value,nvalue) < nvalue)
            fioier = EWBV_PWF;
      }
      else
      {
         if((intu = fiocurf->fassob) == NULL) intu = *(char**)&(fiocurf->ffile);
         ipos = fiocurf->frecrd;
         lpos = fiocurf->fmaxrc;
         if((nvalue + ipos) >= lpos) fioier = EWBV_PWF;
         else
         {   
            cpymem(value,intu+ipos,nvalue);
            fiocurf->frecrd = ipos + nvalue;
         }
      }
   fiowbiv9:
      return fioier;
   }

   /*****************************************************************************/
   /*                                                                           */
   /* FIOWRB: Write FORTRAN Boolean Vector                                      */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* Writes a vector of boolean values to the current output file in           */
   /* accordance with the current format specification. In this context, the    */
   /* term "boolean value" refers to a short logical value. The output display  */
   /* for a logical value consists of a sequence of "fw-1" blanks followed by a */
   /* "T" or a "F", where "fw" is the field width. "T" is user for nonzero      */
   /* values and "F" is used for zero values.                                   */
   /*                                                                           */
   /* Note that this function also supports the FORTRAN 66 convention under     */
   /* which alphabetic information may be stored in the logical values.         */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if there is no error flag set, else an error code. See the general */
   /* discussion if FORTRAN I/O capabilities for a listing of the possible      */
   /* error codes.                                                              */
   /*                                                                           */
   /* See also:                                                                 */
   /*                                                                           */
   /* Referenced by:                                                            */
   /*                                                                           */
   /* Element       Description of use                                          */
   /* -------       ------------------                                          */
   /* FORTRAN       WRITE statement                                             */
   /* fiowvb        Writes a single boolean value                               */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* See detailed steps below.                                                 */
   /*                                                                           */
   /*****************************************************************************/

#ifdef FPROTOTYPE
   int fiowrb(unsigned short* mybool,int nval)
#else
      int fiowrb(mybool,nval)
      unsigned short* mybool;             /* Points to values being written */
   int nval;                         /* Number of values to be written */
#endif
   {
      int i;                    /* Dummy counter moving through vector */
      int nc;                   /* Field width for write */
      unsigned short bv;

      /*------------------------------------------------------------------------*/
      /*                                                                        */
      /* Step 1: Move through the trruth one at a time until all have been      */
      /* written or until an error is encountered. Note that if the error flag  */
      /* is already set when this function is called, then it will exit         */
      /* immediately.                                                           */
      /*                                                                        */
      /*------------------------------------------------------------------------*/

      for(i = 0; (i < nval) && !fioier; i++,mybool++)
      {

         /*---------------------------------------------------------------------*/
         /*                                                                     */
         /* Step 2: Obtain the format specifications until a variable related   */
         /* one is encountered.                                                 */
         /*                                                                     */
         /*---------------------------------------------------------------------*/

         fiofout(0);
         if(fioier) goto ftnwrb9;

         /*---------------------------------------------------------------------*/
         /*                                                                     */
         /* Step 3: If FORTRAN 66 type alphabetic information is being written  */
         /* from the variable, accept it and loop back to step 1. If you wish   */
         /* to impose strict FORTRAN 77 type I/O then comment out the following */
         /* statement and the "else" nesting below it.                          */
         /*                                                                     */
         /*---------------------------------------------------------------------*/

         if(fiofspec == 12) fiowalph((char*)mybool,sizeof(short),fioiwd);
         else
         {

            /*------------------------------------------------------------------*/
            /*                                                                  */
            /* Step 4: If free-field output is being performed display the      */
            /* value as though an "L2" format specification were in effect,     */
            /* else if a "L" specification is found get the specified width,    */
            /* else set an error code.                                          */
            /*                                                                  */
            /*------------------------------------------------------------------*/

            nc = 2;
            if(fiofspec == 0)
            {
               if((fionchar + nc) >= fioffw) fiowtxt();
            }
            else if(fiofspec != 13) fioier = EWRB_IFS;
            else if(fioiwd != 0) nc = fioiwd;

            /*------------------------------------------------------------------*/
            /*                                                                  */
            /* Step 5: If no error has been encountered display a "T" or a "F"  */
            /* right-justified in the field and loop back to step 1.            */
            /*                                                                  */
            /*------------------------------------------------------------------*/

            if(!fioier)
            {
               while(--nc > 0) fiocrec[fionchar++] = ' ';
#ifdef ISUNALIGNED
               bv = get16u((char*)(mybool));
#else
               bv = *mybool;
#endif
               if(bv & 1) fiocrec[fionchar] = 'T';
               else fiocrec[fionchar] = 'F';
               fionchar++;
            }
         }
      }

      /*------------------------------------------------------------------------*/
      /*                                                                        */
      /* Step 6: Either all truth-values have been written or an error has      */
      /* occurred. Check for an error and if one is present perform the user    */
      /* specified action.                                                      */
      /*                                                                        */
      /*------------------------------------------------------------------------*/

   ftnwrb9:
      return fioerror(0);
   }

   /*****************************************************************************/
   /*                                                                           */
   /* FIOWRC: Write FORTRAN Char Vector                                         */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* Writes a vector of character values to the current output file in         */
   /* accordance with the current format specification. In this implementation  */
   /* signed char is derived from the nonstandard FORTRAN types BYTE or         */
   /* INTEGER*1. Therefore, I formatting conventions are assumed. Note that in  */
   /* this function each individual character is assumed to have its own        */
   /* corresponding format specification if a formatted write is being          */
   /* performed.                                                                */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if there is no error flag set, else an error code. See the general */
   /* discussion if FORTRAN I/O capabilities for a listing of the possible      */
   /* error codes.                                                              */
   /*                                                                           */
   /* See also:                                                                 */
   /*                                                                           */
   /* Referenced by:                                                            */
   /*                                                                           */
   /* Element       Description of use                                          */
   /* -------       ------------------                                          */
   /* FORTRAN       WRITE statement                                             */
   /* fiowvc        Write single short value                                    */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* See detailed steps below.                                                 */
   /*                                                                           */
   /*****************************************************************************/

#ifdef FPROTOTYPE
   int fiowrc(char* value,int nval)
#else
      int fiowrc(value,nval)
      char* value;                      /* Points to values to be written */
   int nval;                         /* Number of values to be written */
#endif
   {
      int i;                    /* Dummy counter through the values */
      int nc;                   /* Number of characters in the display */

      /*------------------------------------------------------------------------*/
      /*                                                                        */
      /* Step 1: Move through the values one at a time until all have been      */
      /* written or until an error is encountered. Note that if the error flag  */
      /* is already set when this function is called, then it will exit         */
      /* immediately.                                                           */
      /*                                                                        */
      /*------------------------------------------------------------------------*/

      for(i = 0; (i < nval) && !fioier; i++,value++)
      {

         /*---------------------------------------------------------------------*/
         /*                                                                     */
         /* Step 2: Obtain the format specifications until a variable related   */
         /* one is encountered.                                                 */
         /*                                                                     */
         /*---------------------------------------------------------------------*/

         fiofout(0);
         if(fioier) goto fiowrc9;

         /*---------------------------------------------------------------------*/
         /*                                                                     */
         /* Step 3: If free-field output is being performed, display the value  */
         /* as though an "I5" format specification were in effect. Then loop    */
         /* back to step 1.                                                     */
         /*                                                                     */
         /*---------------------------------------------------------------------*/

         if(fiofspec == 0)
         {
            if(!(fioconv & ZDFLEXF)) fioiwd = 5;
            else fioiwd = 0;
            fiondec = -1;
            if((fionchar + fioiwd) >= fioffw) fiowtxt();
            fioltos((LONG)*value);
         }

         /*---------------------------------------------------------------------*/
         /*                                                                     */
         /* Step 4: If FORTRAN 66 type alphabetic information is being written  */
         /* from the variable, accept it and loop back to step 1. If you wish   */
         /* to inpose strict FORTRAN 77 type I/O then comment out the following */
         /* statement.                                                          */
         /*                                                                     */
         /*---------------------------------------------------------------------*/

         else if(fiofspec == 12) fiowalph((char*)value,1,fioiwd);
         else
         {
            if(fioiwd == 0) fioiwd = 5;
            if(fiofspec == 13)
            {
               nc = fioiwd;
               while(--nc > 0) fiocrec[fionchar++] = ' ';
               if(*value & 1) fiocrec[fionchar] = 'T';
               else fiocrec[fionchar] = 'F';
               fionchar++;
            }

            /*------------------------------------------------------------------*/
            /*                                                                  */
            /* Step 5: If an I format specification was read, write the value   */
            /* accordingly and loop back to step 1.                             */
            /*                                                                  */
            /*------------------------------------------------------------------*/

            else if(fiofspec == 14) fioltos((LONG)*value);

            /*------------------------------------------------------------------*/
            /*                                                                  */
            /* Step 6: Use the generic write value function to convert the      */
            /* value and loop back to step 2.                                   */
            /*                                                                  */
            /*------------------------------------------------------------------*/

            else
            {
               ChrNsig = 6;
               fiowval((double)*value);
            }
         }
      }

      /*------------------------------------------------------------------------*/
      /*                                                                        */
      /* Step 7: Either all values have been written or an error has occurred.  */
      /* Check for an error and if one is present perform the user specified    */
      /* action.                                                                */
      /*                                                                        */
      /*------------------------------------------------------------------------*/

   fiowrc9:
      return fioerror(0);
   }

   /*****************************************************************************/
   /*                                                                           */
   /* FIOWRD: Write FORTRAN Double Precision Vector                             */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* Writes a vector of double precision values to the current output file in  */
   /* accordance with the current format specification. Note that in this       */
   /* function each individual value is assumed to have its own corresponding   */
   /* format specification if a formatted write is being performed.             */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if there is no error flag set, else an error code. See the general */
   /* discussion if FORTRAN I/O capabilities for a listing of the possible      */
   /* error codes.                                                              */
   /*                                                                           */
   /* See also:                                                                 */
   /*                                                                           */
   /* Referenced by:                                                            */
   /*                                                                           */
   /* Element       Description of use                                          */
   /* -------       ------------------                                          */
   /* FORTRAN       WRITE statement                                             */
   /* fiowvd        Writes a single double precision value                      */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* See detailed steps below.                                                 */
   /*                                                                           */
   /*****************************************************************************/

#ifdef FPROTOTYPE
   int fiowrd(double* value,int nval)
#else
      int fiowrd(value,nval)
      double* value;                    /* Points to values being written */
   int nval;                         /* Number of values to be written */
#endif
   {
      int i;

      /*------------------------------------------------------------------------*/
      /*                                                                        */
      /* Step 1: Set the number of significant digits to 13 which is            */
      /* appropriate for a double precision 8 byte value.                       */
      /*                                                                        */
      /*------------------------------------------------------------------------*/

      ChrNsig = 12;

      /*------------------------------------------------------------------------*/
      /*                                                                        */
      /* Step 2: Move through the values one at a time until all have been      */
      /* written or until an error is encountered. Note that if the error flag  */
      /* is already set when this function is called, then it will exit         */
      /* immediately.                                                           */
      /*                                                                        */
      /*------------------------------------------------------------------------*/

      for(i = 0; (i < nval) && !fioier; i++,value++)
      {

         /*---------------------------------------------------------------------*/
         /*                                                                     */
         /* Step 3: Obtain the format specifications until a variable related   */
         /* one is encountered.                                                 */
         /*                                                                     */
         /*---------------------------------------------------------------------*/

         fiofout(0);
         if(fioier) goto ftnwrd9;

         /*---------------------------------------------------------------------*/
         /*                                                                     */
         /* Step 4: If FORTRAN 66 type alphabetic information is being written  */
         /* from the variable, accept it and loop back to step 2. If you wish   */
         /* to inpose strict FORTRAN 77 type I/O then comment out the following */
         /* statement and the following "else".                                 */
         /*                                                                     */
         /*---------------------------------------------------------------------*/

         if(fiofspec == 12) fiowalph((char*)value,sizeof(double),fioiwd);

         /*---------------------------------------------------------------------*/
         /*                                                                     */
         /* Step 5: Use the generic write value function to convert the value   */
         /* and loop back to step 2.                                            */
         /*                                                                     */
         /*---------------------------------------------------------------------*/

#ifdef ISUNALIGNED
         else fiowval(getdbl((char*)(value)));
#else
         else fiowval(*value);
#endif
      }

      /*------------------------------------------------------------------------*/
      /*                                                                        */
      /* Step 6: Either all values have been written or an error has occurred.  */
      /* Check for an error and if one is present perform the user specified    */
      /* action.                                                                */
      /*                                                                        */
      /*------------------------------------------------------------------------*/

   ftnwrd9:
      return fioerror(0);
   }

   /*****************************************************************************/
   /*                                                                           */
   /* FIOWRF: Write FORTRAN Single Precision Vector                             */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* Writes a vector of single precision values to the current output file in  */
   /* accordance with the current format specification. Note that in this       */
   /* function each individual value is assumed to have its own corresponding   */
   /* format specification if a formatted write is being performed.             */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if there is no error flag set, else an error code. See the general */
   /* discussion if FORTRAN I/O capabilities for a listing of the possible      */
   /* error codes.                                                              */
   /*                                                                           */
   /* See also:                                                                 */
   /* Referenced by:                                                            */
   /*                                                                           */
   /* Element       Description of use                                          */
   /* -------       ------------------                                          */
   /* FORTRAN       WRITE statement                                             */
   /* fiowvf        Writes a single single precision value                      */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* See detailed steps below.                                                 */
   /*                                                                           */
   /*****************************************************************************/

#ifdef FPROTOTYPE
   int fiowrf(float* value,int nval)
#else
      int fiowrf(value,nval)
      float* value;                     /* Points to values being written */
   int nval;                         /* Number of values to be written */
#endif
   {
      int i;

      /*------------------------------------------------------------------------*/
      /*                                                                        */
      /* Step 1: Set the number of significant digits to 7 which is appropriate */
      /* for a single precision 4 byte value.                                   */
      /*                                                                        */
      /*------------------------------------------------------------------------*/

      ChrNsig = 7;

      /*------------------------------------------------------------------------*/
      /*                                                                        */
      /* Step 2: Move through the values one at a time until all have been      */
      /* written or until an error is encountered. Note that if the error flag  */
      /* is already set when this function is called, then it will exit         */
      /* immediately.                                                           */
      /*                                                                        */
      /*------------------------------------------------------------------------*/

      for(i = 0; (i < nval) && !fioier; i++,value++)
      {

         /*---------------------------------------------------------------------*/
         /*                                                                     */
         /* Step 3: Obtain the format specifications until a variable related   */
         /* one is encountered.                                                 */
         /*                                                                     */
         /*---------------------------------------------------------------------*/

         fiofout(0);
         if(fioier) goto ftnwrf9;

         /*---------------------------------------------------------------------*/
         /*                                                                     */
         /* Step 4: If FORTRAN 66 type alphabetic information is being written  */
         /* from the variable, accept it and loop back to step 2. If you wish   */
         /* to inpose strict FORTRAN 77 type I/O then comment out the following */
         /* statement and the following "else".                                 */
         /*                                                                     */
         /*---------------------------------------------------------------------*/

         if(fiofspec == 12) fiowalph((char*)value,sizeof(float),fioiwd);

         /*---------------------------------------------------------------------*/
         /*                                                                     */
         /* Step 5: Use the generic write value function to convert the value   */
         /* and loop back to step 2.                                            */
         /*                                                                     */
         /*---------------------------------------------------------------------*/

#ifdef ISUNALIGNED
         else fiowval(getflt((char*)(value)));
#else
         else fiowval(*value);
#endif
      }

      /*------------------------------------------------------------------------*/
      /*                                                                        */
      /* Step 6: Either all values have been written or an error has occurred.  */
      /* Check for an error and if one is present perform the user specified    */
      /* action.                                                                */
      /*                                                                        */
      /*------------------------------------------------------------------------*/

   ftnwrf9:
      return fioerror(0);
   }

   /*****************************************************************************/
   /*                                                                           */
   /* FIOWRI: Write FORTRAN Short Integer Vector                                */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* Writes a vector of short integer values to the current output file        */
   /* in accordance with the current format specification. Note that in this    */
   /* function each individual value is assumed to have its own corresponding   */
   /* format specification, if a formatted write is being performed.            */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if there is no error flag set, else an error code. See the general */
   /* discussion if FORTRAN I/O capabilities for a listing of the possible      */
   /* error codes.                                                              */
   /*                                                                           */
   /* See also:                                                                 */
   /*                                                                           */
   /* Referenced by:                                                            */
   /*                                                                           */
   /* Element       Description of use                                          */
   /* -------       ------------------                                          */
   /* FORTRAN       WRITE statement                                             */
   /* fiowvi        Write single short value                                    */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* See detailed steps below.                                                 */
   /*                                                                           */
   /*****************************************************************************/

#ifdef FPROTOTYPE
   int fiowri(short* value,int nval)
#else
      int fiowri(value,nval)
      short* value;                     /* Points to values to be written */
   int nval;                         /* Number of values to be written */
#endif
   {
      int i;                    /* Dummy counter through the values */
      int nc;                   /* Number of characters in the display */

      /*------------------------------------------------------------------------*/
      /*                                                                        */
      /* Step 1: Move through the values one at a time until all have been      */
      /* written or until an error is encountered. Note that if the error flag  */
      /* is already set when this function is called, then it will exit         */
      /* immediately.                                                           */
      /*                                                                        */
      /*------------------------------------------------------------------------*/

      for(i = 0; (i < nval) && !fioier; i++,value++)
      {

         /*---------------------------------------------------------------------*/
         /*                                                                     */
         /* Step 2: Obtain the format specifications until a variable related   */
         /* one is encountered.                                                 */
         /*                                                                     */
         /*---------------------------------------------------------------------*/

         fiofout(0);
         if(fioier) goto ftnwri9;

         /*---------------------------------------------------------------------*/
         /*                                                                     */
         /* Step 3: If free-field output is being performed, display the value  */
         /* as though a "I7" format specification were in effect. Then loop     */
         /* back to step 1.                                                     */
         /*                                                                     */
         /*---------------------------------------------------------------------*/

         if(fiofspec == 0)
         {
            if(!(fioconv & ZDFLEXF)) fioiwd = 7;
            else fioiwd = 0;
            fiondec = -1;
            if((fionchar + fioiwd) >= fioffw) fiowtxt();
#ifdef ISUNALIGNED
            fioltos((LONG)get16s((char*)(value)));
#else
            fioltos((LONG)*value);
#endif
         }

         /*---------------------------------------------------------------------*/
         /*                                                                     */
         /* Step 4: If FORTRAN 66 type alphabetic information is being written  */
         /* from the variable, accept it and loop back to step 1. If you wish   */
         /* to impose strict FORTRAN 77 type I/O then comment out the following */
         /* statement.                                                          */
         /*                                                                     */
         /*---------------------------------------------------------------------*/

         else if(fiofspec == 12) fiowalph((char*)value,sizeof(short),fioiwd);
         else
         {
            if(fioiwd == 0) fioiwd = 7;
            if(fiofspec == 13)
            {
               nc = fioiwd;
               while(--nc > 0) fiocrec[fionchar++] = ' ';
#ifdef ISUNALIGNED
               if(get16s((char*)(value)) & 1) fiocrec[fionchar] = 'T';
#else
               if(*value & 1) fiocrec[fionchar] = 'T';
#endif
               else fiocrec[fionchar] = 'F';
               fionchar++;
            }

            /*------------------------------------------------------------------*/
            /*                                                                  */
            /* Step 5: If an I format specification was read, write the value   */
            /* accordingly and loop back to step 1.                             */
            /*                                                                  */
            /*------------------------------------------------------------------*/

#ifdef ISUNALIGNED
            else if(fiofspec == 14) fioltos((LONG)get16s((char*)(value)));
#else
            else if(fiofspec == 14) fioltos((LONG)*value);
#endif

            /*------------------------------------------------------------------*/
            /*                                                                  */
            /* Step 6: Use the generic write value function to convert the      */
            /* value and loop back to step 2.                                   */
            /*                                                                  */
            /*------------------------------------------------------------------*/

            else
            {
               ChrNsig = 6;
#ifdef ISUNALIGNED
               fiowval((double)get16s((char*)(value)));
#else
               fiowval((double)*value);
#endif
            }
         }
      }

      /*------------------------------------------------------------------------*/
      /*                                                                        */
      /* Step 7: Either all values have been written or an error has occurred.  */
      /* Check for an error and if one is present perform the user specified    */
      /* action.                                                                */
      /*                                                                        */
      /*------------------------------------------------------------------------*/

   ftnwri9:
      return fioerror(0);
   }

   /*****************************************************************************/
   /*                                                                           */
   /* FIOWRL: Write FORTRAN Long Integer Vector                                 */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* Writes a vector of long integer values to the current output file in      */
   /* accordance with the current format specification. Note that in this       */
   /* function each individual value is assumed to have its own corresponding   */
   /* format specification, if a formatted write is being performed.            */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if there is no error flag set, else an error code. See the general */
   /* discussion if FORTRAN I/O capabilities for a listing of the possible      */
   /* error codes.                                                              */
   /*                                                                           */
   /* See also:                                                                 */
   /*                                                                           */
   /* Referenced by:                                                            */
   /*                                                                           */
   /* Element       Description of use                                          */
   /* -------       ------------------                                          */
   /* FORTRAN       WRITE statement                                             */
   /* fiowvl        Write single long value                                     */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* See detailed steps below.                                                 */
   /*                                                                           */
   /*****************************************************************************/

#ifdef FPROTOTYPE
   int fiowrl(LONG* value,int nval)
#else
      int fiowrl(value,nval)
      LONG* value;                      /* Points to values to be written */
   int nval;                         /* Number of values to be written */
#endif
   {
      int i;                    /* Dummy counter through the values */
      int nc;                   /* Number of characters in the display */

      /*------------------------------------------------------------------------*/
      /*                                                                        */
      /* Step 1: Move through the values one at a time until all have been      */
      /* written or until an error is encountered. Note that if the error flag  */
      /* is already set when this function is called, then it will exit         */
      /* immediately.                                                           */
      /*                                                                        */
      /*------------------------------------------------------------------------*/

      for(i = 0; (i < nval) && !fioier; i++,value++)
      {

         /*---------------------------------------------------------------------*/
         /*                                                                     */
         /* Step 2: Obtain the format specifications until a variable related   */
         /* one is encountered.                                                 */
         /*                                                                     */
         /*---------------------------------------------------------------------*/

         fiofout(0);
         if(fioier) goto ftnwrl9;

         /*---------------------------------------------------------------------*/
         /*                                                                     */
         /* Step 3: If free-field output is being performed, display the value  */
         /* as though a "I12" format specification were in effect. The loop     */
         /* back to step 1.                                                     */
         /*                                                                     */
         /*---------------------------------------------------------------------*/

         if(fiofspec == 0)
         {
            if(!(fioconv & ZDFLEXF)) fioiwd = 12;
            else fioiwd = 0;
            fiondec = -1;
            if((fionchar + fioiwd) >= fioffw) fiowtxt();
#ifdef ISUNALIGNED
            fioltos(get32s((char*)(value)));
#else
            fioltos(*value);
#endif
         }

         /*---------------------------------------------------------------------*/
         /*                                                                     */
         /* Step 4: If FORTRAN 66 type alphabetic information is being written  */
         /* from the variable, accept it and loop back to step 1. If you wish   */
         /* to impose strict FORTRAN 77 type I/O then comment out the following */
         /* statement.                                                          */
         /*                                                                     */
         /*---------------------------------------------------------------------*/

         else if(fiofspec == 12) fiowalph((char*)value,sizeof(LONG),fioiwd);

         /*---------------------------------------------------------------------*/
         /*                                                                     */
         /* Step 6: If an I format specification was read, write the value      */
         /* accordingly and loop back to step 1.                                */
         /*                                                                     */
         /*---------------------------------------------------------------------*/

         else
         {
            if(fiofspec == 14)
            {
               if(fioiwd == 0) fioiwd = 12;
#ifdef ISUNALIGNED
               fioltos(get32s((char*)(value)));
#else
               fioltos(*value);
#endif
            }
            else if(fiofspec == 13)
            {
               if(fioiwd == 0) fioiwd = 2;
               nc = fioiwd;
               while(--nc > 0) fiocrec[fionchar++] = ' ';
#ifdef ISUNALIGNED
               if(get32s((char*)(value)) & 1) fiocrec[fionchar] = 'T';
#else
               if(*value & 1) fiocrec[fionchar] = 'T';
#endif
               else fiocrec[fionchar] = 'F';
               fionchar++;
            }

            /*------------------------------------------------------------------*/
            /*                                                                  */
            /* Step 6: Use the generic write value function to convert the      */
            /* value and loop back to step 2.                                   */
            /*                                                                  */
            /*------------------------------------------------------------------*/

            else
            {
               ChrNsig = 11;
#ifdef ISUNALIGNED
               fiowval((double)get32s((char*)(value)));
#else
               fiowval((double)*value);
#endif
            }
         }
      }

      /*------------------------------------------------------------------------*/
      /*                                                                        */
      /* Step 7: Either all values have been written or an error has occurred.  */
      /* Check for an error and if one is present perform the user specified    */
      /* action.                                                                */
      /*                                                                        */
      /*------------------------------------------------------------------------*/

   ftnwrl9:
      return fioerror(0);
   }

#ifdef FPROTOTYPE
   int fiowrll(longlong* value,int nval)
#else
      int fiowrll(value,nval)
      longlong* value;                  /* Points to values to be written */
   int nval;                         /* Number of values to be written */
#endif
   {
      int i;                    /* Dummy counter through the values */
      int nc;                   /* Number of characters in the display */

      /*------------------------------------------------------------------------*/
      /*                                                                        */
      /* Step 1: Move through the values one at a time until all have been      */
      /* written or until an error is encountered. Note that if the error flag  */
      /* is already set when this function is called, then it will exit         */
      /* immediately.                                                           */
      /*                                                                        */
      /*------------------------------------------------------------------------*/

      for(i = 0; (i < nval) && !fioier; i++,value++)
      {

         /*---------------------------------------------------------------------*/
         /*                                                                     */
         /* Step 2: Obtain the format specifications until a variable related   */
         /* one is encountered.                                                 */
         /*                                                                     */
         /*---------------------------------------------------------------------*/

         fiofout(0);
         if(fioier) goto ftnwrl9;

         /*---------------------------------------------------------------------*/
         /*                                                                     */
         /* Step 3: If free-field output is being performed, display the value  */
         /* as though a "I12" format specification were in effect. The loop     */
         /* back to step 1.                                                     */
         /*                                                                     */
         /*---------------------------------------------------------------------*/

         if(fiofspec == 0)
         {
            if(!(fioconv & ZDFLEXF)) fioiwd = 12;
            else fioiwd = 0;
            fiondec = -1;
            if((fionchar + fioiwd) >= fioffw) fiowtxt();
            fioltos((LONG)(*value));
         }

         /*---------------------------------------------------------------------*/
         /*                                                                     */
         /* Step 4: If FORTRAN 66 type alphabetic information is being written  */
         /* from the variable, accept it and loop back to step 1. If you wish   */
         /* to impose strict FORTRAN 77 type I/O then comment out the following */
         /* statement.                                                          */
         /*                                                                     */
         /*---------------------------------------------------------------------*/

         else if(fiofspec == 12) fiowalph((char*)value,sizeof(longlong),fioiwd);

         /*---------------------------------------------------------------------*/
         /*                                                                     */
         /* Step 6: If an I format specification was read, write the value      */
         /* accordingly and loop back to step 1.                                */
         /*                                                                     */
         /*---------------------------------------------------------------------*/

         else
         {
            if(fiofspec == 14)
            {
               if(fioiwd == 0) fioiwd = 12;
               fioltos((LONG)(*value));
            }
            else if(fiofspec == 13)
            {
               if(fioiwd == 0) fioiwd = 2;
               nc = fioiwd;
               while(--nc > 0) fiocrec[fionchar++] = ' ';
               if(*value & 1) fiocrec[fionchar] = 'T';
               else fiocrec[fionchar] = 'F';
               fionchar++;
            }

            /*------------------------------------------------------------------*/
            /*                                                                  */
            /* Step 6: Use the generic write value function to convert the      */
            /* value and loop back to step 2.                                   */
            /*                                                                  */
            /*------------------------------------------------------------------*/

            else
            {
               ChrNsig = 11;
               fiowval((double)(*value));
            }
         }
      }

      /*------------------------------------------------------------------------*/
      /*                                                                        */
      /* Step 7: Either all values have been written or an error has occurred.  */
      /* Check for an error and if one is present perform the user specified    */
      /* action.                                                                */
      /*                                                                        */
      /*------------------------------------------------------------------------*/

   ftnwrl9:
      return fioerror(0);
   }

   /*****************************************************************************/
   /*                                                                           */
   /* FIOWRS: Write FORTRAN Vector of Strings                                   */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* Writes a sequence of fixed length strings, stored one after another, to   */
   /* the current output file in accordance with the current format specifica-  */
   /* tion. Each string is assumed to have its own corresponding format         */
   /* specification if a formatted write is being performed.                    */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if there is no error flag set, else an error code. See the general */
   /* discussion if FORTRAN I/O capabilities for a listing of the possible      */
   /* error codes.                                                              */
   /*                                                                           */
   /* See also:                                                                 */
   /* Referenced by:                                                            */
   /*                                                                           */
   /* Element       Description of use                                          */
   /* -------       ------------------                                          */
   /* FORTRAN       WRITE statement                                             */
   /* fiowvs        Writes a single string                                      */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* See detailed steps below.                                                 */
   /*                                                                           */
   /*****************************************************************************/

#ifdef FPROTOTYPE
   int fiowrs(char* str,int nstring,int nval)
#else
      int fiowrs(str,nstring,nval)
      char* str;                        /* Points to start of strings */
   int nstring;                      /* Length of each string */
   int nval;                         /* Number of strings to be written */
#endif
   {
      int i;                    /* Dummy counter moving through strings */
      int nc;                   /* Field width for write */

      /*------------------------------------------------------------------------*/
      /*                                                                        */
      /* Step 1: Move through the strings one at a time until all have been     */
      /* written or until an error is encountered. Note that if the error flag  */
      /* is already set when this function is called, then it will exit         */
      /* immediately.                                                           */
      /*                                                                        */
      /*------------------------------------------------------------------------*/

      for(i = 0; (i < nval) && !fioier; i++, str += nstring)
      {

         /*---------------------------------------------------------------------*/
         /*                                                                     */
         /* Step 2: Obtain the format specifications until a variable related   */
         /* one is encountered.                                                 */
         /*                                                                     */
         /*---------------------------------------------------------------------*/

         fiofout(0);
         if(fioier) goto ftnwrs9;

         /*---------------------------------------------------------------------*/
         /*                                                                     */
         /* Step 3: If free-field output is being performed, display the        */
         /* character as though an "An" format specification were in effect,    */
         /* where n is the string length. Then loop back to step 2.             */
         /*                                                                     */
         /*---------------------------------------------------------------------*/

         if(fiofspec == 0)
         {
            if(fionchar > 0 && (fionchar + nstring) > fioffw)
            {
               fiowtxt();
               fiocrec[fionchar++] = ' ';
            }
            fiowalph(str,nstring,nstring);
         }

         /*---------------------------------------------------------------------*/
         /*                                                                     */
         /* Step 4: If an "A" format specification is encountered, determine    */
         /* the field width and write the character. Note that a "A" with no    */
         /* width specified is assumed to be equivalent to a "An" specification */
         /* where n is the string length. Then loop back to step 1.             */
         /*                                                                     */
         /*---------------------------------------------------------------------*/

         else if(fiofspec == 12)
         {
            nc = fioiwd;
            if(!nc) nc = nstring;
            fiowalph(str,nstring,nc);
         }

         /*---------------------------------------------------------------------*/
         /*                                                                     */
         /* Step 5: An invalid format specification has been found. Set the     */
         /* error code accordingly and loop back to step 1.                     */
         /*                                                                     */
         /*---------------------------------------------------------------------*/

         else fioier = EWRS_IFS;
      }

      /*------------------------------------------------------------------------*/
      /*                                                                        */
      /* Step 6: Either all strings have been written or an error has occurred. */
      /* Check for an error and if one is present perform the user specified    */
      /* action.                                                                */
      /*                                                                        */
      /*------------------------------------------------------------------------*/

   ftnwrs9:
      return fioerror(0);
   }

   /*****************************************************************************/
   /*                                                                           */
   /* FIOWRT: Write FORTRAN Truth-Value Vector                                  */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* Writes a vector of truth-values to the current output file in accordance  */
   /* with the current format specification. In this context, the term "truth-  */
   /* value" refers to a long logical value. The output display for a logical   */
   /* value consists of a sequence of "fw-1" blanks followed by a "T" or a "F", */
   /* where "fw" is the field width. "T" is user for nonzero values and "F" is  */
   /* used for zero values.                                                     */
   /*                                                                           */
   /* Note that this function also supports the FORTRAN 66 convention under     */
   /* which alphabetic information may be stored in the logical values.         */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if there is no error flag set, else an error code. See the general */
   /* discussion if FORTRAN I/O capabilities for a listing of the possible      */
   /* error codes.                                                              */
   /*                                                                           */
   /* See also:                                                                 */
   /*                                                                           */
   /* Referenced by:                                                            */
   /*                                                                           */
   /* Element       Description of use                                          */
   /* -------       ------------------                                          */
   /* FORTRAN       WRITE statement                                             */
   /* fiowvt        Writes a single truth-value                                 */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* See detailed steps below.                                                 */
   /*                                                                           */
   /*****************************************************************************/

#ifdef FPROTOTYPE
   int fiowrt(ULONG* mybool,int nval)
#else
      int fiowrt(mybool,nval)
      ULONG* mybool;                      /* Points to values to be written */
   int nval;                         /* Number of values to be written */
#endif
   {
      int i;                    /* Dummy counter moving through vector */
      int nc;                   /* Field width for write */

      /*------------------------------------------------------------------------*/
      /*                                                                        */
      /* Step 1: Move through the trruth one at a time until all have been      */
      /* written or until an error is encountered. Note that if the error flag  */
      /* is already set when this function is called, then it will exit         */
      /* immediately.                                                           */
      /*                                                                        */
      /*------------------------------------------------------------------------*/

      for(i = 0; (i < nval) && !fioier; i++,mybool++)
      {

         /*---------------------------------------------------------------------*/
         /*                                                                     */
         /* Step 2: Obtain the format specifications until a variable related   */
         /* one is encountered.                                                 */
         /*                                                                     */
         /*---------------------------------------------------------------------*/

         fiofout(0);
         if(fioier) goto ftnwrt9;

         /*---------------------------------------------------------------------*/
         /*                                                                     */
         /* Step 3: If FORTRAN 66 type alphabetic information is being written  */
         /* from the variable, accept it and loop back to step 1. If you wish   */
         /* to impose strict FORTRAN 77 type I/O then comment out the following */
         /* statement and the "else" nesting below it.                          */
         /*                                                                     */
         /*---------------------------------------------------------------------*/

         if(fiofspec == 12) fiowalph((char*)mybool,sizeof(LONG),fioiwd);
         else
         {

            /*------------------------------------------------------------------*/
            /*                                                                  */
            /* Step 4: If free-field output is being performed display the      */
            /* value as though an "L2" format specification were in effect,     */
            /* else if a "L" specification is found get the specified width,    */
            /* else set an error code.                                          */
            /*                                                                  */
            /*------------------------------------------------------------------*/

            nc = 2;
            if(fiofspec == 0)
            {
               if((fionchar + nc) >= fioffw) fiowtxt();
            }
            else if(fiofspec != 13) fioier = EWRT_IFS;
            else if(fioiwd != 0) nc = fioiwd;

            /*------------------------------------------------------------------*/
            /*                                                                  */
            /* Step 5: If no error has been encountered display a "T" or a "F"  */
            /* right-justified in the field and loop back to step 1.            */
            /*                                                                  */
            /*------------------------------------------------------------------*/

            if(!fioier)
            {
               while(--nc > 0) fiocrec[fionchar++] = ' ';
#ifdef ISUNALIGNED
               if(get32u((char*)(mybool)) & 1) fiocrec[fionchar] = 'T';
#else
               if(*mybool & 1) fiocrec[fionchar] = 'T';
#endif
               else fiocrec[fionchar] = 'F';
               fionchar++;
            }
         }
      }

      /*------------------------------------------------------------------------*/
      /*                                                                        */
      /* Step 6: Either all truth-values have been written or an error has      */
      /* occurred. Check for an error and if one is present perform the user    */
      /* specified action.                                                      */
      /*                                                                        */
      /*------------------------------------------------------------------------*/

   ftnwrt9:
      return fioerror(0);
   }

   /*****************************************************************************/
   /*                                                                           */
   /* FIOWRU: Write FORTRAN Unsigned Char Vector                                */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* Writes a vector of unsigned character values to the current output file   */
   /* in accordance with the current format specification. In this implementa-  */
   /* tion unsigned char is derived from the nonstandard FORTRAN type LOGICAL*1.*/
   /* Therefore, L formatting conventions are assumed. Note that in this        */
   /* function each individual character is assumed to have its own             */
   /* corresponding format specification if a formatted write is being          */
   /* performed.                                                                */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if there is no error flag set, else an error code. See the general */
   /* discussion if FORTRAN I/O capabilities for a listing of the possible      */
   /* error codes.                                                              */
   /*                                                                           */
   /* See also:                                                                 */
   /*                                                                           */
   /* Referenced by:                                                            */
   /*                                                                           */
   /* Element       Description of use                                          */
   /* -------       ------------------                                          */
   /* FORTRAN       WRITE statement                                             */
   /* fiowvu        Writes a single unsigned character value                    */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* Simply use the standard short boolean write function.                     */
   /*                                                                           */
   /*****************************************************************************/

#ifdef FPROTOTYPE
   int fiowru(unsigned char* c,int nval)
#else
      int fiowru(c,nval)
      unsigned char* c;                 /* Points to characters to be written */
   int nval;                         /* Number of characters to be written */
#endif
   {
      unsigned short ival;      /* Temporary short value */
      int i;                    /* Dummy counter moving though vector */

      for(i = 0; i < nval; i++,c++)
      {
         ival = *c;
         fiowrb(&ival,1);
      }
      return fioerror(0);
   }

   /*****************************************************************************/
   /*                                                                           */
   /* FIOWRX: Write FORTRAN Complex Vector                                      */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* Writes a vector of single precision complex values to the current output  */
   /* file in accordance with the current format specification. Note that in    */
   /* this function each individual component of the complex value is assumed   */
   /* to have its own corresponding floating point format specification if a    */
   /* formatted write is being performed.                                       */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if there is no error flag set, else an error code. See the general */
   /* discussion if FORTRAN I/O capabilities for a listing of the possible      */
   /* error codes.                                                              */
   /*                                                                           */
   /* See also:                                                                 */
   /*                                                                           */
   /* Referenced by:                                                            */
   /*                                                                           */
   /* Element       Description of use                                          */
   /* -------       ------------------                                          */
   /* FORTRAN       WRITE statement                                             */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* Use the fact that each complex value is simply a sequence of two floats   */
   /* and ,thus, simply use the float write function.                           */
   /*                                                                           */
   /*****************************************************************************/

#ifdef FPROTOTYPE
   int fiowrx(complex* value,int nval)
#else
      int fiowrx(value,nval)
      complex* value;                   /* Points to values being written */
   int nval;                         /* Number of values to be written */
#endif
   {
      return fiowrf((float*)value,2*nval);
   }

   /*****************************************************************************/
   /*                                                                           */
   /* FIOWRZ: Write FORTRAN Double Complex Vector                               */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* Writes a vector of double precision complex values to the current output  */
   /* file in accordance with the current format specification. Note that in    */
   /* this function each individual component of the complex value is assumed   */
   /* to have its own corresponding floating point format specification if a    */
   /* formatted write is being performed.                                       */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if there is no error flag set, else an error code. See the general */
   /* discussion if FORTRAN I/O capabilities for a listing of the possible      */
   /* error codes.                                                              */
   /*                                                                           */
   /* See also:                                                                 */
   /*                                                                           */
   /* Referenced by:                                                            */
   /*                                                                           */
   /* Element       Description of use                                          */
   /* -------       ------------------                                          */
   /* FORTRAN       WRITE statement                                             */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* Use the fact that each complex value is simply a sequence of two doubles  */
   /* and, thus, simply use the double write function.                          */
   /*                                                                           */
   /*****************************************************************************/

#ifdef FPROTOTYPE
   int fiowrz(dcomplex* value,int nval)
#else
      int fiowrz(value,nval)
      dcomplex* value;               /* Points to values being written */
   int nval;                      /* Number of values to be written */
#endif
   {
      return fiowrd((double*)value,2*nval);
   }

   /*****************************************************************************/
   /*                                                                           */
   /* FIOWVB: Write FORTRAN Boolean Value                                       */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* Writes a boolean (short logical) value to the current output file in      */
   /* accordance with the current format specification. This function           */
   /* corresponds to the FORTRAN 77 convention in which values and not just l-  */
   /* values can be written.                                                    */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if there is no error flag set, else an error code. See the general */
   /* discussion if FORTRAN I/O capabilities for a listing of the possible      */
   /* error codes.                                                              */
   /*                                                                           */
   /* See also:                                                                 */
   /*                                                                           */
   /* Referenced by:                                                            */
   /*                                                                           */
   /* Element       Description of use                                          */
   /* -------       ------------------                                          */
   /* FORTRAN       WRITE statement                                             */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* Simply pass the address of the boolean value to function "fiowrb" along   */
   /* with a count of 1. This function performs the actual write. Note that for */
   /* short binary constants we revert to "long" which is the assumed FORTRAN   */
   /* storage mode.                                                             */
   /*                                                                           */
   /*****************************************************************************/

#ifdef FPROTOTYPE
   int fiowvb(unsigned short val)
#else
      int fiowvb(val)
      unsigned short val;               /* Boolean value to be written */
#endif
   {
      unsigned short itemp;
      ULONG ltemp;

      if(fiocurf->fstat & FBINARY)
      {
         ltemp = val;
         return fiowbiv(&ltemp,sizeof(LONG));
      }
      else
      {
         itemp = val;
         return fiowrb(&itemp,1);
      }
   }

   /*****************************************************************************/
   /*                                                                           */
   /* FIOWVC: Write FORTRAN Character Value                                     */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* Writes a single character value to the current output file in accordance  */
   /* with the current format specification. This function corresponds to the   */
   /* FORTRAN 77 convention in which values and not l-values can be written.    */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if there is no error flag set, else an error code. See the general */
   /* discussion if FORTRAN I/O capabilities for a listing of the possible      */
   /* error codes.                                                              */
   /*                                                                           */
   /* See also:                                                                 */
   /*                                                                           */
   /* Referenced by:                                                            */
   /*                                                                           */
   /* Element       Description of use                                          */
   /* -------       ------------------                                          */
   /* FORTRAN       WRITE statement                                             */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* Simply pass the address of the character to function "fiowrc" along with  */
   /* a count of 1. This function performs the actual write.                    */
   /*                                                                           */
   /*****************************************************************************/

#ifdef FPROTOTYPE
   int fiowvc(char c)
#else
      int fiowvc(c)
      char c;                           /* Character value to be written */
#endif
   {
      char temp;

      temp = c;
      if(fiocurf->fstat & FBINARY) return fiowbiv(&temp,1);
      else return fiowrc(&temp,1);
   }

   /*****************************************************************************/
   /*                                                                           */
   /* FIOWVD: Write FORTRAN Double Value                                        */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* Writes a double precision value to the current output file in accordance  */
   /* with the current format specification. This function corresponds to the   */
   /* FORTRAN 77 convention in which values and not just l-values can be        */
   /* written.                                                                  */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if there is no error flag set, else an error code. See the general */
   /* discussion if FORTRAN I/O capabilities for a listing of the possible      */
   /* error codes.                                                              */
   /*                                                                           */
   /* See also:                                                                 */
   /*                                                                           */
   /* Referenced by:                                                            */
   /*                                                                           */
   /* Element       Description of use                                          */
   /* -------       ------------------                                          */
   /* FORTRAN       WRITE statement                                             */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* Simply pass the address of the double to function "fiowrd" along with a   */
   /* count of 1. This function performs the actual write.                      */
   /*                                                                           */
   /*****************************************************************************/

#ifdef FPROTOTYPE
   int fiowvd(double val)
#else
      int fiowvd(val)
      double val;                       /* Double value to be written */
#endif
   {
      double temp;

      temp = val;
      if(fiocurf->fstat & FBINARY) return fiowbiv(&temp,sizeof(double));
      else return fiowrd(&temp,1);
   }

   /*****************************************************************************/
   /*                                                                           */
   /* FIOWVF: Write FORTRAN Float Value                                         */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* Writes a single precision value to the current output file in accordance  */
   /* with the current format specification. This function corresponds to the   */
   /* FORTRAN 77 convention in which values and not just l-values can be        */
   /* written.                                                                  */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if there is no error flag set, else an error code. See the general */
   /* discussion if FORTRAN I/O capabilities for a listing of the possible      */
   /* error codes.                                                              */
   /*                                                                           */
   /* See also:                                                                 */
   /*                                                                           */
   /* Referenced by:                                                            */
   /*                                                                           */
   /* Element       Description of use                                          */
   /* -------       ------------------                                          */
   /* FORTRAN       WRITE statement                                             */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* Simply pass the address of the double to function "fiowrd" along with a   */
   /* count of 1. This function performs the actual write. Note that we have to */
   /* be careful here to pass a pointer to a float and not a pointer to a       */
   /* double; therefore, we use a local float variable to contain the value.    */
   /*                                                                           */
   /*****************************************************************************/

#ifdef FPROTOTYPE
   int fiowvf(float val)
#else
      int fiowvf(val)
      float val;                        /* Single value to be written */
#endif
   {
      float fltval;             /* Scratch variable to contain value */

      fltval = (float)(val);
      if(fiocurf->fstat & FBINARY) return fiowbiv(&fltval,sizeof(float));
      else return fiowrf(&fltval,1);
   }

   /*****************************************************************************/
   /*                                                                           */
   /* FIOWVI: Write FORTRAN Short Integer Value                                 */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* Writes a short integer value to the current output file in accordance     */
   /* with the current format specification. This function corresponds to the   */
   /* FORTRAN 77 convention in which values and not just l-values can be        */
   /* written. Note that for binary writes, the short is promoted to long to    */
   /* correspond to the default FORTRAN convention.                             */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if there is no error flag set, else an error code. See the general */
   /* discussion if FORTRAN I/O capabilities for a listing of the possible      */
   /* error codes.                                                              */
   /*                                                                           */
   /* See also:                                                                 */
   /*                                                                           */
   /* Referenced by:                                                            */
   /*                                                                           */
   /* Element       Description of use                                          */
   /* -------       ------------------                                          */
   /* FORTRAN       WRITE statement                                             */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* Simply pass the address of the short integer to function "fiowri" along   */
   /* with a count of 1. This function performs the actual write.               */
   /*                                                                           */
   /*****************************************************************************/

#ifdef FPROTOTYPE
   int fiowvi(short val)
#else
      int fiowvi(val)
      short val;                        /* Short value to be written */
#endif
   {
      short itemp;
      LONG ltemp;

      if(fiocurf->fstat & FBINARY)
      {
         ltemp = val;
         return fiowbiv(&ltemp,sizeof(LONG));
      }
      else
      {
         itemp = val;
         return fiowri(&itemp,1);
      }
   }

   /*****************************************************************************/
   /*                                                                           */
   /* FIOWVL: Write FORTRAN Long Integer Value                                  */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* Writes a long integer value to the current output file in accordance with */
   /* the current format specification. This function corresponds to the        */
   /* FORTRAN 77 convention in which values and not just l-values can be        */
   /* written.                                                                  */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if there is no error flag set, else an error code. See the general */
   /* discussion if FORTRAN I/O capabilities for a listing of the possible      */
   /* error codes.                                                              */
   /*                                                                           */
   /* See also:                                                                 */
   /*                                                                           */
   /* Referenced by:                                                            */
   /*                                                                           */
   /* Element       Description of use                                          */
   /* -------       ------------------                                          */
   /* FORTRAN       WRITE statement                                             */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* Simply pass the address of the long integer to function "fiowrl" along    */
   /* with a count of 1. This function performs the actual write.               */
   /*                                                                           */
   /*****************************************************************************/

#ifdef FPROTOTYPE
   int fiowvl(LONG val)
#else
      int fiowvl(val)
      LONG val;                         /* Long value to be written */
#endif
   {
      LONG temp;

      temp = val;
      if(fiocurf->fstat & FBINARY) return fiowbiv(&temp,sizeof(LONG));
      else return fiowrl(&temp,1);
   }

#ifdef FPROTOTYPE
   int fiowvll(longlong val)
#else
      int fiowvll(val)
      longlong val;                         /* Long value to be written */
#endif
   {
      longlong temp;

      temp = val;
      if(fiocurf->fstat & FBINARY) return fiowbiv(&temp,sizeof(longlong));
      else return fiowrll(&temp,1);
   }

   /*****************************************************************************/
   /*                                                                           */
   /* FIOWVS: Write FORTRAN String Value                                        */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* Writes a single character string to the current output file in accordance */
   /* with the current format specification. This function corresponds to the   */
   /* FORTRAN 77 convention in which values and not l-values can be written.    */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if there is no error flag set, else an error code. See the general */
   /* discussion if FORTRAN I/O capabilities for a listing of the possible      */
   /* error codes.                                                              */
   /*                                                                           */
   /* See also:                                                                 */
   /*                                                                           */
   /* Referenced by:                                                            */
   /*                                                                           */
   /* Element       Description of use                                          */
   /* -------       ------------------                                          */
   /* FORTRAN       WRITE statement                                             */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* Simply pass the address of the string to function "fiowrs" along with a   */
   /* count of 1. This function performs the actual write.                      */
   /*                                                                           */
   /*****************************************************************************/

#ifdef FPROTOTYPE
   int fiowvs(char* str,int nstring)
#else
      int fiowvs(str,nstring)
      char* str;                        /* Character string to be written */
   int nstring;                      /* Length of string */
#endif
   {
      if(fiocurf->fstat & FBINARY) return fiowbiv(str,nstring);
      else return fiowrs(str,nstring,1);
   }

   /*****************************************************************************/
   /*                                                                           */
   /* FIOWVT: Write FORTRAN Truth Value                                         */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* Writes a truth (long logical) value to the current output file in         */
   /* accordance with the current format specification. This function           */
   /* corresponds to the FORTRAN 77 convention in which values and not just     */
   /* l-values can be written.                                                  */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if there is no error flag set, else an error code. See the general */
   /* discussion if FORTRAN I/O capabilities for a listing of the possible      */
   /* error codes.                                                              */
   /*                                                                           */
   /* See also:                                                                  */
   /*                                                                           */
   /* Referenced by:                                                            */
   /*                                                                           */
   /* Element       Description of use                                          */
   /* -------       ------------------                                          */
   /* FORTRAN       WRITE statement                                             */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* Simply pass the address of the truth value to function "fiowrt" along     */
   /* with a count of 1. This function performs the actual write.               */
   /*                                                                           */
   /*****************************************************************************/

#ifdef FPROTOTYPE
   int fiowvt(ULONG val)
#else
      int fiowvt(val)
      ULONG val;                        /* Truth value to be written */
#endif
   {
      ULONG temp;

      temp = val;
      if(fiocurf->fstat & FBINARY) return fiowbiv(&temp,sizeof(LONG));
      else return fiowrt(&temp,1);
   }

   /*****************************************************************************/
   /*                                                                           */
   /* FIOWVU: Write FORTRAN Character Value                                     */
   /*                                                                           */
   /* Copyright 1988-94 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Synopsis:                                                                 */
   /*                                                                           */
   /* Writes a single character value to the current output file in accordance  */
   /* with the current format specification. This function corresponds to the   */
   /* FORTRAN 77 convention in which values and not l-values can be written.    */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if there is no error flag set, else an error code. See the         */
   /* general discussion if FORTRAN I/O capabilities for a listing of           */
   /* the possible error codes.                                                 */
   /*                                                                           */
   /*****************************************************************************/

#ifdef FPROTOTYPE
   int fiowvu(unsigned char c)
#else
      int fiowvu(c)
      unsigned char c;        /* Character value to be written */
#endif
   {
      unsigned char temp;

      temp = c;
      if(fiocurf->fstat & FBINARY) return fiowbiv(&temp,1);
      else return fiowru(&temp,1);
   }

   /*****************************************************************************/
   /*                                                                           */
   /* FIOWVX: Write FORTRAN Complex Value                                       */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* Writes a single precision complex value to the current output file in     */
   /* accordance with the current format specification. This function           */
   /* corresponds to the FORTRAN 77 convention in which values and not just     */
   /* l-values can be written.                                                  */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if there is no error flag set, else an error code. See the general */
   /* discussion if FORTRAN I/O capabilities for a listing of the possible      */
   /* error codes.                                                              */
   /*                                                                           */
   /* See also:                                                                 */
   /*                                                                           */
   /* Referenced by:                                                            */
   /*                                                                           */
   /* Element       Description of use                                          */
   /* -------       ------------------                                          */
   /* FORTRAN       WRITE statement                                             */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* Simply pass the address of the value to function "fiowrf" along with a    */
   /* count of 2. This function performs the actual write.                      */
   /*                                                                           */
   /*****************************************************************************/

#ifdef FPROTOTYPE
   int fiowvx(complex val)
#else
      int fiowvx(val)
      complex val;                      /* Single value to be written */
#endif
   {
      float temp;

      temp = val.cr;
      if(fiocurf->fstat & FBINARY) fiowbiv(&temp,sizeof(float));
      else if(!fionfmt)
      {
         ChrNsig = 7;
         fiofspec = fiondec = fioiwd = 0;
         fiocrec[fionchar++] = '(';
         fiowval((double)temp);
         fiocrec[fionchar++] = ',';
      }
      else fiowrf(&temp,1);
      temp = val.ci;
      if(fiocurf->fstat & FBINARY) return fiowbiv(&temp,sizeof(float));
      else if(!fionfmt)
      {
         fiofspec = fiondec = fioiwd = 0;
         fiowval((double)temp);
         fiocrec[fionchar++] = ')';
         return fioerror(0);
      }
      else return fiowrf(&temp,1);
   }

   /*****************************************************************************/
   /*                                                                           */
   /* FIOWVZ: Write FORTRAN Complex Value                                       */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* Writes a double precision complex value to the current output file in     */
   /* accordance with the current format specification. This function           */
   /* corresponds to the FORTRAN 77 convention in which values and not just     */
   /* l-values can be written.                                                  */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if there is no error flag set, else an error code. See the general */
   /* discussion if FORTRAN I/O capabilities for a listing of the possible      */
   /* error codes.                                                              */
   /*                                                                           */
   /* See also:                                                                 */
   /*                                                                           */
   /* Referenced by:                                                            */
   /*                                                                           */
   /* Element       Description of use                                          */
   /* -------       ------------------                                          */
   /* FORTRAN       WRITE statement                                             */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* Simply pass the address of the value to function "fiowrf" along with a    */
   /* count of 2. This function performs the actual write.                      */
   /*                                                                           */
   /*****************************************************************************/

#ifdef FPROTOTYPE
   int fiowvz(dcomplex val)
#else
      int fiowvz(val)
      dcomplex val;                  /* Single value to be written */
#endif
   {
      double temp;

      temp = val.dr;
      if(fiocurf->fstat & FBINARY) fiowbiv(&temp,sizeof(double));
      else if(!fionfmt)
      {
         ChrNsig = 12;
         fiofspec = fiondec = fioiwd = 0;
         fiocrec[fionchar++] = '(';
         fiowval(temp);
         fiocrec[fionchar++] = ',';
      }
      else fiowrd(&temp,1);
      temp = val.di;
      if(fiocurf->fstat & FBINARY) return fiowbiv(&temp,sizeof(double));
      else if(!fionfmt)
      {
         fiofspec = fiondec = fioiwd = 0;
         fiowval(temp);
         fiocrec[fionchar++] = ')';
         return fioerror(0);
      }
      else return fiowrd(&temp,1);
   }

   /*****************************************************************************/
   /*                                                                           */
   /* Copyright (C) 1988-98 PROMULA Development Corp., All Rights Reserved      */
   /*                                                                           */
   /* ftnwrit: FORTRAN Write Statement                                          */
   /*                                                                           */
   /* Author(s): Fred Goodman (FG)                                              */
   /*                                                                           */
   /* Creation date: 06/14/1988                                                 */
   /*                                                                           */
   /* FG 12/01/97 Update: Corrected the retreival of pointer arguments so that  */
   /* pointers are being correctly typed, rather than using a generic character */
   /* pointer. This change was made necessary by the Data General platform      */
   /* which has a special representation for character pointers.                */
   /*                                                                           */
   /* Purpose: Executes the FORTRAN WRITE statement when translated via a non-  */
   /* optimized user bias. In addition to the logical unit number,this function */
   /* takes a variable number of parameters which specify the actual data to be */
   /* supplied to the detailed write functions. The parameter type codes passed */
   /* to this function are as follows:                                          */
   /*                                                                           */
   /* Code   Parameter  Description of Use                                      */
   /* ----   ---------  ------------------                                      */
   /*  1     LONG*      Points to an error return variable                      */
   /*  2     ---        Indicates that list-directed I/O is being performed     */
   /*  3     char**,int Points to a full FORMAT specification                   */
   /*  4     char*      Points to a FORMAT string                               */
   /*  5     int        Specifies a record number                               */
   /*  9     ---        Specifies that more operations are to be performed      */
   /* 10     int        Specifies that a series of operations are desired       */
   /* 11                Introduces a short integer value                        */
   /* 12                Introduces a double precision value                     */
   /* 13                Introduces a short logical value                        */
   /* 14                Introduces a char value                                 */
   /* 15                Introduces a long value                                 */
   /* 16                Introduces a float value                                */
   /* 17                Introduces a long logical value                         */
   /* 18                Introduces an unsigned char value                       */
   /* 19                Introduces a complex value                              */
   /* 20                Introduces a string value                               */
   /* 21                Introduces a C-string value                             */
   /*                                                                           */
   /* Return value: A zero if the open went well, else an error code. See the   */
   /* general discussion of FORTRAN I/O capabilities for a listing of the       */
   /* possible error codes.                                                     */
   /*                                                                           */
   /*****************************************************************************/

#ifdef VARUNIXV
   int ftnwrit(va_alist) va_dcl      /* FORTRAN Write Statement */
#else
      int ftnwrit(int lun,...)
#endif
      {
         va_list argptr;
         int oper;
         int len;
         int ier;                  /* Error processing type code */
         char** fmt;               /* Format string pointer */
         char* str;                /* Dummy pointer */
         int ilun;                 /* If 0,file structure not established */
         LONG* iostat;             /* Error return status code */
         short sval;               /* Dummy short integer value */
         unsigned short uval;      /* Dummy unsigned short integer value */
         double dval;              /* Dummy floating point value */
         LONG lval;                /* Dummy long integer value */
         ULONG ulval;              /* Dummy unsigned long value */
         complex xval;             /* Dummy complex value */
         int ndo;                  /* Implied doloop count */
         char* ifile;              /* Pointer to internal file storage */
         int nifile;               /* Internal file record length */
         int ival;
         short* ShortPtr;          /* Short value pointer */
         double* DoublePtr;        /* Double precision value pointer */
         unsigned short* BooleanPtr;/* Boolean value pointer */
         char* CharacterPtr;       /* Character value pointer */
         LONG* LongPtr;            /* Long value pointer */
         float* FloatPtr;          /* Single precision value pointer */
         ULONG* LogicalPtr;        /* Logical value pointer */
         unsigned char* BytePtr;   /* Byte value pointer */
         complex* ComplexPtr;      /* Complex value pointer */
         char* StringPtr;          /* String value pointer */
         longlong* LongLongPtr;
         longlong llval;

         /*------------------------------------------------------------------------*/
         /*                                                                        */
         /* Step 1: Initialize the file structure creation variable in accordance  */
         /* with whether this is a MORE call or an intial call, and intialize the  */
         /* machinery needed to deal with the variable number of arguments.        */
         /*                                                                        */
         /*------------------------------------------------------------------------*/

#ifdef VARUNIXV
         int lun;
         va_start(argptr);
         lun = va_arg(argptr,int);
#else
         va_start(argptr,lun);
#endif
         ilun = mode;
         nifile = len = 0;
         fioier = fioerch = 0;
         fiostat = NULL;
         ifile = NULL;
         if(lun == NAMED) {
            ifile = va_arg(argptr,char*);
            nifile = va_arg(argptr,int);
            fioname(ifile,nifile,0);
            if(fioier == 0) lun = fiocurf->funit;
         }
         /*------------------------------------------------------------------------*/
         /*                                                                        */
         /* Step 2: If this is an intialization call obtain the first parameter    */
         /* from the argument list and if it is an error status specification      */
         /* establish it accordingly, then obtain the next parameter from the list.*/
         /* Else if this is a MORE call, the first parameter code is contained in  */
         /* the "lun" parameter.                                                   */
         /*                                                                        */
         /*------------------------------------------------------------------------*/

         if(!mode) {
            iomode = 0;
            if(lun == INTERNAL) {       /* This is an internal file */
               ifile = va_arg(argptr,char*);
               nifile = va_arg(argptr,int);
            }
            oper = va_arg(argptr,int);
            if(oper == 1) {
               iostat = va_arg(argptr,LONG*);
               ier = va_arg(argptr,int);
               fiostatus(iostat,ier);
               if(fioier != 0) {
                  va_end(argptr);
                  return fioerror(1);
               }
               oper = va_arg(argptr,int);
            }
         }
         else oper = lun;

         /*------------------------------------------------------------------------*/
         /*                                                                        */
         /* Step 3: A parameter type code has been obtained from the parameter     */
         /* list. If it is not part of the iolist, process it and obtain and       */
         /* additional parameters until an iolist parameter is found.              */
         /*                                                                        */
         /*------------------------------------------------------------------------*/

      ftnwrit3:
         if(oper > 8 || oper == 0) {
            ndo = 0;
            if(!ilun) {
               if(!iomode) {
                  if(lun == INTERNAL) fiointu(ifile,nifile,5);
                  else fiolun(lun,5);
               }
               else {
                  if(lun == OUTPUT) fiostio(4);
                  else if(lun == CONSOLE) fiostio(1);
                  else if(lun == PRINTER) fiostio(3);
                  else if(lun == INTERNAL) fiointu(ifile,nifile,4);
                  else fiolun(lun,4);
               }
            }
            goto ftnwrit4;
         }
         switch(oper) {
         case 2:                        /* List-directed WRITE is being requested. */
            iomode = 1;
            break;
         case 3:                        /* Full format specification being intro'd */
            iomode = 1;
            fmt = va_arg(argptr,char**);
            len = va_arg(argptr,int);
            fiofini(fmt,len);
            break;
         case 4:                        /* A variable format is being introduced */
            iomode = 1;
            str = va_arg(argptr,char*);
            len = va_arg(argptr,int);
            fiovfini((char**)str,len);
            break;
         case 5:                        /* A particular record has been requested */
            if(lun == INTERNAL) fiointu(ifile,nifile,5);
            else fiolun(lun,5);
            ilun = 1;
            len = va_arg(argptr,int);
            fiorec(len);
            break;
         }
         oper = va_arg(argptr,int);
         goto ftnwrit3;

         /*------------------------------------------------------------------------*/
         /*                                                                        */
         /* Step 4: An actual value is being written. The information following    */
         /* the parameter and the actual utility to be used depends upon the       */
         /* context. For binary writes there is a pointer of the indicated type    */
         /* followed by the number of elements of that type to be transfered. For  */
         /* single coded writes there is a value of the indicated type, and for    */
         /* multiple coded writes, there is a pointer.                             */
         /*                                                                        */
         /*------------------------------------------------------------------------*/

      ftnwrit4:
         if(oper == 0 || oper == 9) {   /* The WRITE operation has been completed. */
            va_end(argptr);
            if(oper == 9) {
               mode = 1;
               return 0;
            }
            fiowln();
            mode = 0;
            return fioerror(1);
         }
         if(oper == 10) {
            ndo = va_arg(argptr,int);
            oper = va_arg(argptr,int);
            goto ftnwrit4;
         }
         oper -= 11;
         if(iomode != 0) goto ftnwrit6;
         switch(oper) {
         case 0:                        /* Introduces a short integer value */
            str = (char*)(va_arg(argptr,short*));
            len = sizeof(short);
            break;
         case 1:                        /* Introduces a double precision value */
            str = (char*)(va_arg(argptr,double*));
            len = sizeof(double);
            break;
         case 2:                        /* Introduces a short logical value */
            str = (char*)(va_arg(argptr,unsigned short*));
            len = sizeof(unsigned short);
            break;
         case 3:                        /* Introduces a char value */
            str = va_arg(argptr,char*);
            len = sizeof(char);
            break;
         case 4:                        /* Introduces a long value */
            str = (char*)(va_arg(argptr,LONG*));
            len = sizeof(LONG);
            break;
         case 5:                        /* Introduces a float value */
            str = (char*)(va_arg(argptr,float*));
            len = sizeof(float);
            break;
         case 6:                        /* Introduces a long logical value */
            str = (char*)(va_arg(argptr,ULONG*));
            len = sizeof(ULONG);
            break;
         case 7:                        /* Introduces an unsigned char value */
            str = (char*)(va_arg(argptr,unsigned char*));
            len = sizeof(unsigned char);
            break;
         case 8:                        /* Introduces a complex value */
            str = (char*)(va_arg(argptr,complex*));
            len = sizeof(complex);
            break;
         case 9:                        /* Introduces a string value */
            str = va_arg(argptr,char*);
            len = va_arg(argptr,int);
            break;
         case 12:
            str = (char*)(va_arg(argptr,longlong*));
            len = sizeof(longlong);
            break;
         default:
         case 10:                       /* Introduces a C-string value */
            str = va_arg(argptr,char*);
            len = 1;
            break;
         }
         ival = va_arg(argptr,int);
         fiowbiv(str,len*ival);
         oper = va_arg(argptr,int); 
         goto ftnwrit4;
      ftnwrit6:
         if(ndo == 0) goto ftnwrit7;
         switch(oper) {
         case 0:                        /* Introduces a short integer value */
            ShortPtr = va_arg(argptr,short*);
            fiowri(ShortPtr,ndo);
            break;
         case 1:                        /* Introduces a double precision value */
            DoublePtr = va_arg(argptr,double*);
            fiowrd(DoublePtr,ndo);
            break;
         case 2:                        /* Introduces a short logical value */
            BooleanPtr = va_arg(argptr,unsigned short*);
            fiowrb(BooleanPtr,ndo);
            break;
         case 3:                        /* Introduces a char value */
            CharacterPtr = va_arg(argptr,char*);
            fiowrc(CharacterPtr,ndo);
            break;
         case 4:                        /* Introduces a long value */
            LongPtr = va_arg(argptr,LONG*);
            fiowrl(LongPtr,ndo);
            break;
         case 5:                        /* Introduces a float value */
            FloatPtr = va_arg(argptr,float*);
            fiowrf(FloatPtr,ndo);
            break;
         case 6:                        /* Introduces a long logical value */
            LogicalPtr = va_arg(argptr,ULONG*);
            fiowrt(LogicalPtr,ndo);
            break;
         case 7:                        /* Introduces an unsigned char value */
            BytePtr = va_arg(argptr,unsigned char*);
            fiowru(BytePtr,ndo);
            break;
         case 8:                        /* Introduces a complex value */
            ComplexPtr = va_arg(argptr,complex*);
            fiowrx(ComplexPtr,ndo);
            break;
         case 9:                        /* Introduces a string value */
            StringPtr = va_arg(argptr,char*);
            len = va_arg(argptr,int);
            fiowrs(StringPtr,len,ndo);
            break;
         case 10:                       /* Introduces a C-string value */
            str = va_arg(argptr,char*);
            break;
         case 12:                      /* Introduces a longlong value */
            LongLongPtr = va_arg(argptr,longlong*);
            fiowrll(LongLongPtr,ndo);
            break;
         }
         ndo = 0;
         oper = va_arg(argptr,int); 
         goto ftnwrit4;
      ftnwrit7:
         switch(oper) {
         case 0:                        /* Introduces a short integer value */
            sval = (short) va_arg(argptr,int);
            fiowvi(sval);
            break;
         case 1:                        /* Introduces a double precision value */
#ifdef LONGDBL
            dval = va_arg(argptr,long double);
#else
            dval = va_arg(argptr,double);
#endif
            fiowvd(dval);
            break;
         case 2:                        /* Introduces a short logical value */
            uval = (unsigned short) va_arg(argptr,unsigned int);
            fiowvb(uval);
            break;
         case 3:                        /* Introduces a char value */
            ival = va_arg(argptr,int);
            fiowvc((char)(ival));
            break;
         case 4:                        /* Introduces a long value */
            lval = va_arg(argptr,LONG);
            fiowvl(lval);
            break;
         case 5:                        /* Introduces a float value */
#ifdef LONGDBL
            dval = va_arg(argptr,long double);
#else
            dval = va_arg(argptr,double);
#endif
            fiowvf((float) dval);
            break;
         case 6:                        /* Introduces a long logical value */
            ulval = va_arg(argptr,ULONG);
            fiowvt(ulval);
            break;
         case 7:                        /* Introduces an unsigned char value */
            sval = (short) va_arg(argptr,int);
            fiowvu((unsigned char) sval);
            break;
         case 8:                        /* Introduces a complex value */
#ifdef DGEPLAT
            xval.cr = va_arg(argptr,float);
            xval.ci = va_arg(argptr,float);
#else
            xval = va_arg(argptr,complex);
#endif
            fiowvx(xval);
            break;
         case 9:                        /* Introduces a string value */
            str = va_arg(argptr,char*);
            len = va_arg(argptr,int);
            fiowvs(str,len);
            break;
         case 10:                       /* Introduces a C-string value */
            str = va_arg(argptr,char*);
            len = strlen(str);
            fiowvs(str,len);
            break;
         case 12:                       /* Introduces a long value */
            llval = va_arg(argptr,longlong);
            fiowvll(llval);
            break;
         }
         oper = va_arg(argptr,int); 
         goto ftnwrit4;
      }


   /*
     Define the constants used in the translation of the auxilliary I/O statements
     for the C and FORTRAN user biases.
   */
#ifndef TRUE
#define true          1
#endif
#ifndef FALSE
#define false         0
#endif
#define OPEN          ftnopen
#define PAUSE         ftnpause
#define REWIND        ftnrew
#define FLUSH         ftnflush
#define ENDFILE       ftnwef
#define BACKSPACE     ftnback
#define CLOSE         ftnclose
// #define DELETE        ftndelet
#define LOCKFILE      ftnlock
#define UNLOCKFILE    ftnunlock
#define RNAMELIST     rnmlist
#define WNAMELIST     wnmlist
#define Stop          ftnstop
#define LUN           ftnlun
#define IOSTAT        1
#define FILEN         2
#define STATUS        3
#define ACCESS        4
#define FORM          5
#define RECL          6
#define BLANK         7
#define READONLY      8
#define SHARED        9
#define RECTYPE      10
#define CARRCON      11
#define ASVARIABLE   12
#define RECORDS      13
#define ANYUNIT      14
#define ASBUFFER     15
#define BLOCKSIZE    16
#define OTHER        17
#define INDEXKEY     18
#define ORGANIZATION 19
#define USEROPEN     20
#define DISPOSE      21
#define BUFFERCOUNT  22
#define ERRORLOG     23
#define RECFM        24
#define MAXREC       25
#define EXCLUSIVE    28
   /*
     Define the constants used in the READ and WRITE statements.
   */
#define  WRITE        ftnwrit
#define  READ         ftnread
#define  NAMED        -9994
#define  CONSOLE      -9995
#define  PRINTER      -9996
#define  INTERNAL     -9997
#define  INPUT        -9998
#define  OUTPUT       -9999
#define  LISTIO       2
#define  FMT          3
#define  VFMT         4
#define  REC          5
#define  MORE         9
#define  DO          10
#define  INT2        11
#define  REAL8       12
#define  LOG2        13
#define  BYTE        14
#define  INT4        15
#define  REAL4       16
#define  LOG4        17
#define  LOG1        18
#define  CMPLX       19
#define  STRG        20
#define  CSTR        21
#define  DCMPLX      22
   /*
     Define conversion constants for trig functions
   */
#define F_PI   3.141592654
#define F_HPI  1.570796327
#define F_R2D  57.295779513
#define F_D2R  1.745329252E-02
#define D_PI   3.141592653589793238462643
#define D_HPI  1.570796326794896619231322
#define D_R2D  57.295779513082320876798155
#define D_D2R  1.7453292519943295769237E-02
   /*
     Define the "float" math functions
   */
#define fifabs(x)       ((float)fabs((double)(x)))
#define fifacos(x)      ((float)acos((double)(x)))
#define dacosd(x)       (acos(x)*D_R2D)
#define fifaint(x)      ((float)fifdint((double)(x)))
#define fiflog10(x)     ((float)log10((double)(x)))

#define fiflog(x)       ((float)log((double)(x)))
#define fifamax1(x,y)   ((float)fifdmax1((double)(x),(double)(y)))
#define fifamin1(x,y)   ((float)fifdmin1((double)(x),(double)(y)))
#define fifamod(x,y)    ((float)fifdmod((double)(x),(double)(y)))
#define fifanint(x)     ((float)fifdnint((double)(x)))
#define fifasin(x)      ((float)asin((double)(x)))
#define dasind(x)       (asin(x)*D_R2D)
#define fifatan(x)      ((float)atan((double)(x)))
#define fifatan2(x,y)   ((float)atan2((double)(x),(double)(y)))
#define datand(x)       (atan(x)*D_R2D)
#define datan2d(x,y)    (atan2(x,y)*D_R2D)
#define atan2d(x,y)     ((float)atan2((float)(x),(float)(y))*F_R2D)
#define fifcos(x)       ((float)cos((double)(x)))
#define fifcosh(x)      ((float)cosh((double)(x)))
#define fifdim(x,y)     ((float)fifddim((double)(x),(double)(y)))
#define fifexp(x)       ((float)exp((double)(x)))
#define fifiaint(x)     fifidint((double)(x))
#define fifi2aint(x)    fifi2dint((double)(x))
#define fifpow(x,y)     ((float)pow((double)(x),(double)(y)))
#define fifsign(x,y)    ((float)fifdsign((double)(x),(double)(y)))
#define fifsin(x)       ((float)sin((double)(x)))
#define fifsinh(x)      ((float)sinh((double)(x)))
#define fifsqrt(x)      ((float)sqrt((double)(x)))
#define fifdsind(x)     sin(x*D_D2R)
#define fifdcosd(x)     cos(x*D_D2R)
#define fifdtand(x)     tan(x*D_D2R)
   /*
     Define the unaligned memory functions
   */
#ifdef FPROTOTYPE
   USHORT get16u(char* unl);
   SHORT get16s(char* unl);
   ULONG get32u(char* unl);
   LONG get32s(char* unl);
   float getflt(char* unl);
   double getdbl(char* unl);
   complex getcpx(char* unl);
   dcomplex getdpx(char* unl);
   void put16u(char* unl,USHORT val);
   void put16s(char* unl,SHORT val);
   void put32u(char* unl,ULONG val);
   void put32s(char* unl,LONG val);
   void putflt(char* unl,float val);
   void putdbl(char* unl,double val);
   void putcpx(char* unl,complex val);
   void putdpx(char* unl,dcomplex val);
#else
   USHORT get16u();
   SHORT get16s();
   ULONG get32u();
   LONG get32s();
   float getflt();
   double getdbl();
   complex getcpx();
   dcomplex getdpx();
   void put16u();
   void put16s();
   void put32u();
   void put32s();
   void putflt();
   void putdbl();
   void putcpx();
   void putdpx();
#endif

   /*
     ;/hdr/ ************************************************************************
     ;
     ; Copyright 1986-1998 Promula Development Corporation ALL RIGHTS RESERVED
     ;
     ; name of module: FioInit.c -- Fortan I/o Initialization
     ;
     ; This function is called as the first executable statement of any "main"
     ; function produced by the translator. It contains the declarations of the
     ; variables needed by the FORTRAN I/O system.
     ;
     ; The parameters "argc" and "argv" are simply the command line arguments
     ; passed to the main function and then passed here. Via the dialect
     ; description file (keyword 27) the user has the option to specify the
     ; command switchs explicitely. In this case, those switchs are contained
     ; in the third parameter (separated by blanks).
     ;
     ; This function scans the command line strings looking for arguments of the
     ; following form:
     ;
     ; Argument      Description
     ; --------      -----------
     ; Cnumber       Establishes the runtime conventions code:
     ;               0 = Standard conventions
     ;               1 = Prime extended character set
     ; Inumber       Assigns the unit whose number is indicated to standard
     ;               input.
     ; Onumber       Assigns the unit whose number is indicated to standard
     ;               output.
     ; Tnumber       Assigns the unit whose number is indicated to be a terminal
     ;               -- i.e. reads from standard input, wwites to standard
     ;               output
     ;
     ; This function does call the "BLOCK DATA" function which is supplied with
     ; this function library as a dummy routine.
     ;
     ; global symbols defined:
     ;
     ; useful notes and assumptions:
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; edit 06/17/98 FKG: Changed names of global symbols to conform to the 
     ;                    Promula version 7.08 API naming conventions. 
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00
     ;
     ;/hdr/ ************************************************************************
   */
#define CHARTYPE                  /* Gives access to character type info */
#define PRIOFUNC                  /* Gives access to PROMULA I/O functions */
#define RAWMFUNC                  /* Gives access to raw memory functions */
#define STRGFUNC                  /* Gives access to the string functions */
   // #include "platform.h"             /* Define the platform hosting this code */
   // #include "pfclib.h"               /* Define PROMULA.FORTRAN library symbols */
   // #include "chrapi.h"               /* Define character manipulation interface */

#ifdef BO2PLAT
#include <float.h>
#endif
#ifdef CPLUSPLUS
#include "ftnfile.hpp"            /* Define the C++ ftnfle class */
   ftnfile Globalftn;
   ftnfile::ftnfile() 
      {
         return; 
      }
#endif

#ifdef FPROTOTYPE
   void  (*FioCloseOperation)(void) = NULL; 
   // char* (*FioConvertName)(char* fname,int leng) = NULL;
   char* (*FioLocalName)(char* fname,int leng,int unit) = NULL;
#else
   void  (*FioCloseOperation)() = NULL; 
   char* (*FioConvertName)() = NULL;
   char* (*FioLocalName)() = NULL;
#endif

   int fioblkn = 0;                  /* Current blanks convention */
   int fiobsex = LITTLENDER;         /* Little-end byte sex as true or false */
   char fiocrec[FIOBMAX];            /* Raw coded input record */
   FIOFILE* fiocurf = NULL;          /* Pointer to current FORTRAN file */
   int fioerch = 0;                  /* Specifies presence of error checking */
   int fioconv = 0;                  /* General dialect convention flags */
   int fioier = 0;                   /* Code for actual error encountered */
   LONG* fiostat = NULL;             /* Returns an error code or zero */
   int fioefc = 'E';                 /* E-format exponent character */
   int fioefw = 0;                   /* E-format exponent width */
   int fioeman = 0;                  /* E-format mantissa width */
   int fioemax = 6;                  /* E-format maximum threshhold */
   int fioemin = -1;                 /* E-format minimum threshhold */
   int fioscal = 0;                  /* Current value of P scaling factor */
   int fioicol = 0;                  /* Current input record column number */
   int fioerc = 0;                   /* Error return code */
   int fioiwd = 0;                   /* Current format width specification */
   int fiolcol = -1;                 /* Input record last column number */
   int fiolfil = ' ';                /* Numeric display left-fill character */
   int fionchar = 0;                 /* Number of output characters */
   int fiondec = 0;                  /* Number of decimal places */
   int fionopn = 0;                  /* Number of file structures used */
   int fioinsc = '?';                /* Insignificant digit display character */
   int fioffw = 81;                  /* Record width for free-form output */
   FIOFILE fioopnf[FIOMAX];          /* Describes all currently open files */
#ifdef MPEPLAT    
   char fioprefx[20] = "FTN";        /* Missing file name prefix */
#else
   char fioprefx[20] = "fort.";      /* Missing file name prefix */
#endif
   int fioplus = ' ';                /* Numeric display plus sign character */
   LONG fiorecn = 0;                 /* Current record number desired */
   LONG fiorecl = -1;                /* Current unformated record length */
   int fiorpeat = 0;                 /* Current free-form repeat count */
   txtfile fiosinp = nultxtf;        /* Current terminal input file */
   txtfile fiosout = nultxtf;        /* Current terminal output file */
   txtfile fioscon = nultxtf;
   const char **comlin = NULL;             /* Pointer to command line arguments */
   int ncomlin = 0;                  /* Number of command line arguments */

#ifdef FPROTOTYPE
   void ftnblkd(void);
#else
   extern void ftnblkd();            /* BLOCK DATA subprogram */
#endif

#ifdef FPROTOTYPE
   void ftnini(int argc,const char* argv[],char* defarg)
#else
      void ftnini(argc,argv,defarg)
      int argc;                         /* Number of command line arguments */
      const char* argv[ ];                    /* Pointers to command line arguments */
      char* defarg;                     /* Default arguments */
#endif
   {
      int i;                    /* Dummy counter */
      int ity;                  /* Flag type code */
      int ival;                 /* Dummy value */
      int nc;
      char* s;                  /* Dummy character counter */

      /*------------------------------------------------------------------------*/
      /*                                                                        */
      /* Step 1: Initialize the global variables.                               */
      /*                                                                        */
      /*------------------------------------------------------------------------*/

      fiocrec[0] = '\0';
      ChrFlags = CHR_LEADZERO;
      comlin = argv;
      ncomlin = argc;
      fioscon = SCONSOL;

      /*------------------------------------------------------------------------*/
      /*                                                                        */
      /* Step 2: If we are on a platform which by default does not accept zero  */
      /* divides, but which can be told to do so, then do it now. Many FORTRAN  */
      /* programs assume that they may divide by zero with no problems.         */
      /*                                                                        */
      /*------------------------------------------------------------------------*/

#ifdef BO2PLAT
      _control87(MCW_EM,MCW_EM);
#endif
#ifdef WTCPLAT
      _grow_handles(50);
#endif

      /*------------------------------------------------------------------------*/
      /*                                                                        */
      /* Step 3: If there is no default set of command line arguments, then     */
      /* create one from the actual arguments, using the raw input buffer as a  */
      /* temporary storage area.                                                */
      /*                                                                        */
      /*------------------------------------------------------------------------*/

      ival = 0;
      if(defarg != NULL) {
         ival = strlen(defarg);
         cpymem(defarg,fiocrec,ival);    
         fiocrec[ival++] = ' ';
      }
      for(i = 1; i < argc; i++) {
         nc = strlen(argv[i]);
         cpymem(argv[i],fiocrec+ival,nc+1);
         ival += nc;
      }
      fiocrec[ival] = '\0';
      defarg = fiocrec;

      /*------------------------------------------------------------------------*/
      /*                                                                        */
      /* Step 4: Scan the command line arguments looking for those parameters   */
      /* which are relevant to this function -- i.e. the standard unit          */
      /* assignments. If found, store the specified unit numbers.               */
      /*                                                                        */
      /*------------------------------------------------------------------------*/

      s = defarg;
      while(*s) {
         if(*s == 'i' || *s == 'I' || *s == 'o' || *s == 'O' || *s == 't' ||
            *s == 'T' || *s == 'c' || *s == 'C')
         {
            ity = *s;
            ival = 0;
            for(;;) {
               s++;
               if(ChrTypeFlags[(int)*s] & CNUMBER) ival = ival * 10 + ChrInformation[(int)*s];
               else break;
            }
            if(ity == 'c' || ity == 'C') fioconv = ival;
            else {
               fiocurf = fioopnf + fionopn;
               fionopn++;
               fiocurf->funit = ival;
               fiocurf->fname[0] = '\0';
               fiocurf->frsize = fiocurf->frecrd = fiocurf->fstat = 0;
#ifdef BLANKNULL
               fiocurf->fstat |= FBLANK;
#endif
               fiocurf->fstat1 = FCOMLINE;
               if(ity == 'i' || ity == 'I')
               {
                  *(txtfile*)&(fiocurf->ffile) = SINPUT;
                  fiocurf->fstat |= FEXIST;
               }
               else if(ity == 't' || ity == 'T')
               {
                  fiocurf->fstat |= FTERMIO;
                  *(txtfile*)&(fiocurf->ffile) = nultxtf;
               }
               else *(txtfile*)&(fiocurf->ffile) = SOUTPUT;
               if(((fioconv & ZDFLEXF) != 0) && (ival == 6))
               {
                  fiocurf->fstat |= FCARRIG;
               }
            }
         }
         else if(*s == ' ') s++;
         else while(*s != '\0' && *s != ' ') s++;
      }

      /*------------------------------------------------------------------------*/
      /*                                                                        */
      /* Step 5: Call the block data subprogram to initialize any application   */
      /* specific global variables.                                             */
      /*                                                                        */
      /*------------------------------------------------------------------------*/

      ftnblkd();
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; ChrInitialize: Initialize Character Processing
     ;
     ; Synopsis of Service:
     ;
     ; #include "chrapi.h"                  Character processing interface
     ;
     ; void ChrInitialize()                 Initialize character processing
     ;
     ; Description of Service:
     ;
     ; This service initializes the basic character description variables. It must
     ; be called once and only once by the main thread of any application intending
     ; to do any character processing.
     ;
     ; In particular, this service initializes a series of character control
     ; vectors which are needed to perform character manipulation at the platform
     ; independent system level:
     ;
     ; Vector           Description of use
     ; ------           ------------------
     ; ChrAlphabet      The uppercase alphabetic characters in sequence
     ; ChrDigits        The numeric digit characters in sequence
     ; ChrInformation   The information control value for each possible code
     ; ChrTypeFlags     The character type flags for each possible code
     ;
     ; The normal C character services assume C conventions and provide no natural
     ; way of converting arbitrary character display codes into their values. Note
     ; that this service makes NO assumptions about the actual internal values for
     ; the characters. It works without change with EBCDIC or ASCII or whatever
     ; representation is being used.
     ;
     ; Properties of Service: None
     ;
     ; Return Value from Service: None
     ; 
     ;/doc/ ************************************************************************
   */

   // #ifndef platform_h                /* Is this a standalone file */
   // #define CHARTYPE                  /* Gives access to character type info */
   // #include "platform.h"             /* Define the platform hosting this code */
   // #include "chrapi.h"               /* Character manipulation Api */
   // #endif /* platform_h */

   int ChrBufferSize = 4096;         /* Size character storage circular buffer */
   int ChrEndComment = 0;
   int ChrEfmtchr = 'E';             /* E-format exponent character */
   int ChrEfmtwth = 0;               /* E-format field width */
   int ChrEmantisa = 1;              /* Width of in E-format mantissa */
   int ChrEmaxfmt = 6;               /* Upper threshhold for E-format */
   int ChrEminfmt = -1;              /* Lower threshhold for E-format */
   int ChrError = 0;                 /* Character conversion error code */
   int ChrFlags = 0;                 /* Numeric conversion control flags */
   const char* ChrFullNames[12] =          /* Full month names */
      {
         "January",
         "February",
         "March",
         "April",
         "May",
         "June",
         "July",
         "August",
         "September",
         "October",
         "November",
         "December"
      };
   int ChrIhist = 0;                 /* Error return support information */
   int ChrIsdble = 0;                /* Indicates if double precision */
   UBYTE* ChrListEntry = NULL;       /* Points to last found list entry */
   UBYTE* ChrListInfo = NULL;        /* Points to last found list information */
   int ChrNdec = 0;                  /* Number of decimal places read */
   int ChrNsig = 13;                 /* Maximum number of significant digits */
   int ChrWidth = 0;                 /* Report column width */
   int ChrNonsig = '0';              /* Character to represent nonsig digits */
   int ChrPlus =  ' ';               /* Plus sign character */
   int ChrLfill = ' ';               /* Left side numeric fill character */
   int ChrOverFlow = '*';            /* Overflow character */
   int ChrCrossOver = 60;
   int ChrModulo = 10;
   int ChrTen[5] = {10,100,1000,10000,100000};
   char ChrAlphabet[26] =            /* Alphabetic characters in sequence */
      {
         'A','B','C','D','E','F','G','H','I','J','K','L','M',
         'N','O','P','Q','R','S','T','U','V','W','X','Y','Z'
      };
   char ChrDigits[10] =              /* Numeric characters in sequence */
      {
         '0','1','2','3','4','5','6','7','8','9'
      };
   UBYTE ChrMasks[8] =               /* Character delimeter masks */
      {            
         1,2,4,8,16,32,64,128
      };
   char ChrMonthNames[ ] = 
      {
         'J','a','n', 'F','e','b', 'M','a','r', 'A','p','r', 'M','a','y',
         'J','u','n', 'J','u','l', 'A','u','g', 'S','e','p', 'O','c','t',
         'N','o','v', 'D','e','c'
      };
   char Chr3DayNames[ ] = 
      {
         'S','u','n', 'M','o','n', 'T','u','e', 'W','e','d', 'T','h','r',
         'F','r','i', 'S','a','t'
      };
   const char* ChrFullDayNames[ ] = 
      {
         "Sunday",
         "Monday",
         "Tuesday",
         "Wednesday",
         "Thursday",
         "Friday",
         "Saturday"
      }; 

   UBYTE ChrInformation[256] = {    /* Character information */
      /*  0 */ 0,
      /*  1 */ 0,
      /*  2 */ 0,
      /*  3 */ 0,
      /*  4 */ 0,
      /*  5 */ 0,
      /*  6 */ 0,
      /*  7 */ 0,
      /*  8 */ 0,
      /*  9 */ 0,
      /* 10 */ 0,
      /* 11 */ 0,
      /* 12 */ 0,
      /* 13 */ 0,
      /* 14 */ 0,
      /* 15 */ 0,
      /* 16 */ 0,
      /* 17 */ 0,
      /* 18 */ 0,
      /* 19 */ 0,
      /* 20 */ 0,
      /* 21 */ 0,
      /* 22 */ 0,
      /* 23 */ 0,
      /* 24 */ 0,
      /* 25 */ 0,
      /* 26 */ 0,
      /* 27 */ 0,
      /* 28 */ 0,
      /* 29 */ 0,
      /* 30 */ 0,
      /* 31 */ 0,
      /* 32 */ 45,
      /* 33 */ 53,
      /* 34 */ 49,
      /* 35 */ 0,
      /* 36 */ 43,
      /* 37 */ 54,
      /* 38 */ 0,
      /* 39 */ 48,
      /* 40 */ 41,
      /* 41 */ 42,
      /* 42 */ 39,
      /* 43 */ 37,
      /* 44 */ 46,
      /* 45 */ 38,
      /* 46 */ 47,
      /* 47 */ 40,
      /* 48 */ 0,
      /* 49 */ 1,
      /* 50 */ 2,
      /* 51 */ 3,
      /* 52 */ 4,
      /* 53 */ 5,
      /* 54 */ 6,
      /* 55 */ 7,
      /* 56 */ 8,
      /* 57 */ 9,
      /* 58 */ 50,
      /* 59 */ 51,
      /* 60 */ 0,
      /* 61 */ 44,
      /* 62 */ 0,
      /* 63 */ 0,
      /* 64 */ 0,
      /* 65 */ 97,
      /* 66 */ 98,
      /* 67 */ 99,
      /* 68 */ 100,
      /* 69 */ 101,
      /* 70 */ 102,
      /* 71 */ 103,
      /* 72 */ 104,
      /* 73 */ 105,
      /* 74 */ 106,
      /* 75 */ 107,
      /* 76 */ 108,
      /* 77 */ 109,
      /* 78 */ 110,
      /* 79 */ 111,
      /* 80 */ 112,
      /* 81 */ 113,
      /* 82 */ 114,
      /* 83 */ 115,
      /* 84 */ 116,
      /* 85 */ 117,
      /* 86 */ 118,
      /* 87 */ 119,
      /* 88 */ 120,
      /* 89 */ 121,
      /* 90 */ 122,
      /* 91 */ 55,
      /* 92 */ 52,
      /* 93 */ 56,
      /* 94 */ 0,
      /* 95 */ 0,
      /* 96 */ 0,
      /* 97  a */ 1,
      /* 98  b */ 2,
      /* 99  c */ 3,
      /* 100 d */ 4,
      /* 101 e */ 5,
      /* 102 f */ 6,
      /* 103 g */ 7,
      /* 104 h */ 8,
      /* 105 i */ 9,
      /* 106 j */ 10,
      /* 107 k */ 11,
      /* 108 l */ 12,
      /* 109 m */ 13,
      /* 110 n */ 14,
      /* 111 o */ 15,
      /* 112 p */ 16,
      /* 113 q */ 17,
      /* 114 r */ 18,
      /* 115 */ 19,
      /* 116 */ 20,
      /* 117 */ 21,
      /* 118 */ 22,
      /* 119 */ 23,
      /* 120 */ 24,
      /* 121 */ 25,
      /* 122 */ 26,
      /* 123 */ 0,
      /* 124 */ 0,
      /* 125 */ 0,
      /* 126 */ 0,
      /* 127 */ 0,
      /* 128 */ 0,
      /* 129 */ 0,
      /* 130 */ 0,
      /* 131 */ 0,
      /* 132 */ 0,
      /* 133 */ 0,
      /* 134 */ 0,
      /* 135 */ 0,
      /* 136 */ 0,
      /* 137 */ 0,
      /* 138 */ 0,
      /* 139 */ 0,
      /* 140 */ 0,
      /* 141 */ 0,
      /* 142 */ 0,
      /* 143 */ 0,
      /* 144 */ 0,
      /* 145 */ 0,
      /* 146 */ 0,
      /* 147 */ 0,
      /* 148 */ 0,
      /* 149 */ 0,
      /* 150 */ 0,
      /* 151 */ 0,
      /* 152 */ 0,
      /* 153 */ 0,
      /* 154 */ 0,
      /* 155 */ 0,
      /* 156 */ 0,
      /* 157 */ 0,
      /* 158 */ 0,
      /* 159 */ 0,
      /* 160 */ 0,
      /* 161 */ 0,
      /* 162 */ 0,
      /* 163 */ 0,
      /* 164 */ 0,
      /* 165 */ 0,
      /* 166 */ 0,
      /* 167 */ 0,
      /* 168 */ 0,
      /* 169 */ 0,
      /* 170 */ 0,
      /* 171 */ 0,
      /* 172 */ 0,
      /* 173 */ 0,
      /* 174 */ 0,
      /* 175 */ 0,
      /* 176 */ 0,
      /* 177 */ 0,
      /* 178 */ 0,
      /* 179 */ 0,
      /* 180 */ 0,
      /* 181 */ 0,
      /* 182 */ 0,
      /* 183 */ 0,
      /* 184 */ 0,
      /* 185 */ 0,
      /* 186 */ 0,
      /* 187 */ 0,
      /* 188 */ 0,
      /* 189 */ 0,
      /* 190 */ 0,
      /* 191 */ 0,
      /* 192 */ 0,
      /* 193 */ 0,
      /* 194 */ 0,
      /* 195 */ 0,
      /* 196 */ 0,
      /* 197 */ 0,
      /* 198 */ 0,
      /* 199 */ 0,
      /* 200 */ 0,
      /* 201 */ 0,
      /* 202 */ 0,
      /* 203 */ 0,
      /* 204 */ 0,
      /* 205 */ 0,
      /* 206 */ 0,
      /* 207 */ 0,
      /* 208 */ 0,
      /* 209 */ 0,
      /* 210 */ 0,
      /* 211 */ 0,
      /* 212 */ 0,
      /* 213 */ 0,
      /* 214 */ 0,
      /* 215 */ 0,
      /* 216 */ 0,
      /* 217 */ 0,
      /* 218 */ 0,
      /* 219 */ 0,
      /* 220 */ 0,
      /* 221 */ 0,
      /* 222 */ 0,
      /* 223 */ 0,
      /* 224 */ 0,
      /* 225 */ 0,
      /* 226 */ 0,
      /* 227 */ 0,
      /* 228 */ 0,
      /* 229 */ 0,
      /* 230 */ 0,
      /* 231 */ 0,
      /* 232 */ 0,
      /* 233 */ 0,
      /* 234 */ 0,
      /* 235 */ 0,
      /* 236 */ 0,
      /* 237 */ 0,
      /* 238 */ 0,
      /* 239 */ 0,
      /* 240 */ 0,
      /* 241 */ 0,
      /* 242 */ 0,
      /* 243 */ 0,
      /* 244 */ 0,
      /* 245 */ 0,
      /* 246 */ 0,
      /* 247 */ 0,
      /* 248 */ 0,
      /* 249 */ 0,
      /* 250 */ 0,
      /* 251 */ 0,
      /* 252 */ 0,
      /* 253 */ 0,
      /* 254 */ 0,
      /* 255 */ 0
   };
   UBYTE ChrTypeFlags[256] = {    /* Character type flags */
      /*    0 */ 0,
      /*    1 */ 0,
      /*    2 */ 0,
      /*    3 */ 0,
      /*    4 */ 0,
      /*    5 */ 0,
      /*    6 */ 0,
      /*    7 */ 0,
      /*    8 */ 0,
      /*    9 */ CWSPACE,
      /*   10 */ 0,
      /*   11 */ 0,
      /*   12 */ 0,
      /*   13 */ 0,
      /*   14 */ 0,
      /*   15 */ 0,
      /*   16 */ 0,
      /*   17 */ 0,
      /*   18 */ 0,
      /*   19 */ 0,
      /*   20 */ 0,
      /*   21 */ 0,
      /*   22 */ 0,
      /*   23 */ 0,
      /*   24 */ 0,
      /*   25 */ 0,
      /*   26 */ 0,
      /*   27 */ 0,
      /*   28 */ 0,
      /*   29 */ 0,
      /*   30 */ 0,
      /*   31 */ 0,
      /*   32 */ CSYMBOL | CWSPACE,
      /* ! 33 */ CSYMBOL,
      /* " 34 */ CSYMBOL | CQUOTEC,
      /* # 35 */ 0,
      /* $ 36 */ CSYMBOL,
      /* % 37 */ CSYMBOL,
      /* & 38 */ CSYMBOL,
      /* ' 39 */ CSYMBOL | CQUOTEC, 
      /* ( 40 */ CSYMBOL,
      /* ) 41 */ CSYMBOL,
      /* * 42 */ CSYMBOL,
      /* + 43 */ CSYMBOL,
      /* , 44 */ CSYMBOL,
      /* - 45 */ CSYMBOL,
      /* . 46 */ CSYMBOL,
      /* / 47 */ CSYMBOL,
      /* 0 48 */ CIDENTC | CNUMBER,
      /* 1 49 */ CIDENTC | CNUMBER,
      /* 2 50 */ CIDENTC | CNUMBER,
      /* 3 51 */ CIDENTC | CNUMBER,
      /* 4 52 */ CIDENTC | CNUMBER,
      /* 5 53 */ CIDENTC | CNUMBER,
      /* 6 54 */ CIDENTC | CNUMBER,
      /* 7 55 */ CIDENTC | CNUMBER,
      /* 8 56 */ CIDENTC | CNUMBER,
      /* 9 57 */ CIDENTC | CNUMBER,
      /* : 58 */ CSYMBOL,
      /* ; 59 */ CSYMBOL,
      /* < 60 */ 0,
      /* = 61 */ CSYMBOL,
      /* > 62 */ 0,
      /* ? 63 */ 0,
      /* @ 64 */ 0,
      /* A 65 */ CLETTER | CUPPERC | CIDENTC,
      /* B 66 */ CLETTER | CUPPERC | CIDENTC,
      /* C 67 */ CLETTER | CUPPERC | CIDENTC,
      /* D 68 */ CLETTER | CUPPERC | CIDENTC,
      /* E 69 */ CLETTER | CUPPERC | CIDENTC,
      /* F 70 */ CLETTER | CUPPERC | CIDENTC,
      /* G 71 */ CLETTER | CUPPERC | CIDENTC,
      /* H 72 */ CLETTER | CUPPERC | CIDENTC,
      /* I 73 */ CLETTER | CUPPERC | CIDENTC,
      /* J 74 */ CLETTER | CUPPERC | CIDENTC,
      /* K 75 */ CLETTER | CUPPERC | CIDENTC,
      /* L 76 */ CLETTER | CUPPERC | CIDENTC,
      /* M 77 */ CLETTER | CUPPERC | CIDENTC,
      /* N 78 */ CLETTER | CUPPERC | CIDENTC,
      /* O 79 */ CLETTER | CUPPERC | CIDENTC,
      /* P 80 */ CLETTER | CUPPERC | CIDENTC,
      /* Q 81 */ CLETTER | CUPPERC | CIDENTC,
      /* R 82 */ CLETTER | CUPPERC | CIDENTC,
      /* S 83 */ CLETTER | CUPPERC | CIDENTC,
      /* T 84 */ CLETTER | CUPPERC | CIDENTC,
      /* U 85 */ CLETTER | CUPPERC | CIDENTC,
      /* V 86 */ CLETTER | CUPPERC | CIDENTC,
      /* W 87 */ CLETTER | CUPPERC | CIDENTC,
      /* X 88 */ CLETTER | CUPPERC | CIDENTC,
      /* Y 89 */ CLETTER | CUPPERC | CIDENTC,
      /* Z 90 */ CLETTER | CUPPERC | CIDENTC,
      /* [ 91 */ CSYMBOL,
      /* \ 92 */ CSYMBOL,
      /* ] 93 */ CSYMBOL,
      /* ^ 94 */ 0,
      /* _ 95 */ CIDENTC,
      /* ` 96 */ 0,
      /* a 97 */ CLETTER | CLOWERC | CIDENTC,
      /* b 98 */ CLETTER | CLOWERC | CIDENTC,
      /* c 99 */ CLETTER | CLOWERC | CIDENTC,
      /* d 100 */ CLETTER | CLOWERC | CIDENTC,
      /* e 101 */ CLETTER | CLOWERC | CIDENTC,
      /* f 102 */ CLETTER | CLOWERC | CIDENTC,
      /* g 103 */ CLETTER | CLOWERC | CIDENTC,
      /* h 104 */ CLETTER | CLOWERC | CIDENTC,
      /* i 105 */ CLETTER | CLOWERC | CIDENTC,
      /* j 106 */ CLETTER | CLOWERC | CIDENTC,
      /* k 107 */ CLETTER | CLOWERC | CIDENTC,
      /* l 108 */ CLETTER | CLOWERC | CIDENTC,
      /* m 109 */ CLETTER | CLOWERC | CIDENTC,
      /* n 110 */ CLETTER | CLOWERC | CIDENTC,
      /* o 111 */ CLETTER | CLOWERC | CIDENTC,
      /* p 112 */ CLETTER | CLOWERC | CIDENTC,
      /* q 113 */ CLETTER | CLOWERC | CIDENTC,
      /* 114 */ CLETTER | CLOWERC | CIDENTC,
      /* 115 */ CLETTER | CLOWERC | CIDENTC,
      /* 116 */ CLETTER | CLOWERC | CIDENTC,
      /* 117 */ CLETTER | CLOWERC | CIDENTC,
      /* 118 */ CLETTER | CLOWERC | CIDENTC,
      /* 119 */ CLETTER | CLOWERC | CIDENTC,
      /* 120 */ CLETTER | CLOWERC | CIDENTC,
      /* 121 */ CLETTER | CLOWERC | CIDENTC,
      /* 122 */ CLETTER | CLOWERC | CIDENTC,
      /* 123 */ 0,
      /* 124 */ 0,
      /* 125 */ 0,
      /* 126 */ 0,
      /* 127 */ 0,
      /* 128 */ 0,
      /* 129 */ 0,
      /* 130 */ 0,
      /* 131 */ 0,
      /* 132 */ 0,
      /* 133 */ 0,
      /* 134 */ 0,
      /* 135 */ 0,
      /* 136 */ 0,
      /* 137 */ 0,
      /* 138 */ 0,
      /* 139 */ 0,
      /* 140 */ 0,
      /* 141 */ 0,
      /* 142 */ 0,
      /* 143 */ 0,
      /* 144 */ 0,
      /* 145 */ 0,
      /* 146 */ 0,
      /* 147 */ 0,
      /* 148 */ 0,
      /* 149 */ 0,
      /* 150 */ 0,
      /* 151 */ 0,
      /* 152 */ 0,
      /* 153 */ 0,
      /* 154 */ 0,
      /* 155 */ 0,
      /* 156 */ 0,
      /* 157 */ 0,
      /* 158 */ 0,
      /* 159 */ 0,
      /* 160 */ 0,
      /* 161 */ 0,
      /* 162 */ 0,
      /* 163 */ 0,
      /* 164 */ 0,
      /* 165 */ 0,
      /* 166 */ 0,
      /* 167 */ 0,
      /* 168 */ 0,
      /* 169 */ 0,
      /* 170 */ 0,
      /* 171 */ 0,
      /* 172 */ 0,
      /* 173 */ 0,
      /* 174 */ 0,
      /* 175 */ 0,
      /* 176 */ 0,
      /* 177 */ 0,
      /* 178 */ 0,
      /* 179 */ 0,
      /* 180 */ 0,
      /* 181 */ 0,
      /* 182 */ 0,
      /* 183 */ 0,
      /* 184 */ 0,
      /* 185 */ 0,
      /* 186 */ 0,
      /* 187 */ 0,
      /* 188 */ 0,
      /* 189 */ 0,
      /* 190 */ 0,
      /* 191 */ 0,
      /* 192 */ 0,
      /* 193 */ 0,
      /* 194 */ 0,
      /* 195 */ 0,
      /* 196 */ 0,
      /* 197 */ 0,
      /* 198 */ 0,
      /* 199 */ 0,
      /* 200 */ 0,
      /* 201 */ 0,
      /* 202 */ 0,
      /* 203 */ 0,
      /* 204 */ 0,
      /* 205 */ 0,
      /* 206 */ 0,
      /* 207 */ 0,
      /* 208 */ 0,
      /* 209 */ 0,
      /* 210 */ 0,
      /* 211 */ 0,
      /* 212 */ 0,
      /* 213 */ 0,
      /* 214 */ 0,
      /* 215 */ 0,
      /* 216 */ 0,
      /* 217 */ 0,
      /* 218 */ 0,
      /* 219 */ 0,
      /* 220 */ 0,
      /* 221 */ 0,
      /* 222 */ 0,
      /* 223 */ 0,
      /* 224 */ 0,
      /* 225 */ 0,
      /* 226 */ 0,
      /* 227 */ 0,
      /* 228 */ 0,
      /* 229 */ 0,
      /* 230 */ 0,
      /* 231 */ 0,
      /* 232 */ 0,
      /* 233 */ 0,
      /* 234 */ 0,
      /* 235 */ 0,
      /* 236 */ 0,
      /* 237 */ 0,
      /* 238 */ 0,
      /* 239 */ 0,
      /* 240 */ 0,
      /* 241 */ 0,
      /* 242 */ 0,
      /* 243 */ 0,
      /* 244 */ 0,
      /* 245 */ 0,
      /* 246 */ 0,
      /* 247 */ 0,
      /* 248 */ 0,
      /* 249 */ 0,
      /* 250 */ 0,
      /* 251 */ 0,
      /* 252 */ 0,
      /* 253 */ 0,
      /* 254 */ 0,
      /* 255 */ 0
   };

   UBYTE ChrDelimeters[256] = {    /* Character delimeter sets */
      /*  0 */ 255,
      /*  1 */ 0,
      /*  2 */ 0,
      /*  3 */ 0,
      /*  4 */ 0,
      /*  5 */ 0,
      /*  6 */ 0,
      /*  7 */ 0,
      /*  8 */ 0,
      /*  9 */ 0,
      /* 10 */ 0,
      /* 11 */ 0,
      /* 12 */ 0,
      /* 13 */ 0,
      /* 14 */ 0,
      /* 15 */ 0,
      /* 16 */ 0,
      /* 17 */ 0,
      /* 18 */ 0,
      /* 19 */ 0,
      /* 20 */ 0,
      /* 21 */ 0,
      /* 22 */ 0,
      /* 23 */ 0,
      /* 24 */ 0,
      /* 25 */ 0,
      /* 26 */ 0,
      /* 27 */ 0,
      /* 28 */ 0,
      /* 29 */ 0,
      /* 30 */ 0,
      /* 31 */ 0,
      /* 32 */ 0,
      /* 33 */ 0,
      /* 34 */ 0,
      /* 35 */ 0,
      /* 36 */ 0,
      /* 37 */ 0,
      /* 38 */ 0,
      /* 39 */ 0,
      /* 40 */ 0,
      /* 41 */ 0,
      /* 42 */ 0,
      /* 43 */ 0,
      /* 44 */ 0,
      /* 45 */ 0,
      /* 46 */ 0,
      /* 47 */ 0,
      /* 48 */ 0,
      /* 49 */ 0,
      /* 50 */ 0,
      /* 51 */ 0,
      /* 52 */ 0,
      /* 53 */ 0,
      /* 54 */ 0,
      /* 55 */ 0,
      /* 56 */ 0,
      /* 57 */ 0,
      /* 58 */ 0,
      /* 59 */ 0,
      /* 60 */ 0,
      /* 61 */ 0,
      /* 62 */ 0,
      /* 63 */ 0,
      /* 64 */ 0,
      /* 65 */ 0,
      /* 66 */ 0,
      /* 67 */ 0,
      /* 68 */ 0,
      /* 69 */ 0,
      /* 70 */ 0,
      /* 71 */ 0,
      /* 72 */ 0,
      /* 73 */ 0,
      /* 74 */ 0,
      /* 75 */ 0,
      /* 76 */ 0,
      /* 77 */ 0,
      /* 78 */ 0,
      /* 79 */ 0,
      /* 80 */ 0,
      /* 81 */ 0,
      /* 82 */ 0,
      /* 83 */ 0,
      /* 84 */ 0,
      /* 85 */ 0,
      /* 86 */ 0,
      /* 87 */ 0,
      /* 88 */ 0,
      /* 89 */ 0,
      /* 90 */ 0,
      /* 91 */ 0,
      /* 92 */ 0,
      /* 93 */ 0,
      /* 94 */ 0,
      /* 95 */ 0,
      /* 96 */ 0,
      /* 97 */ 0,
      /* 98 */ 0,
      /* 99 */ 0,
      /* 100 */ 0,
      /* 101 */ 0,
      /* 102 */ 0,
      /* 103 */ 0,
      /* 104 */ 0,
      /* 105 */ 0,
      /* 106 */ 0,
      /* 107 */ 0,
      /* 108 */ 0,
      /* 109 */ 0,
      /* 110 */ 0,
      /* 111 */ 0,
      /* 112 */ 0,
      /* 113 */ 0,
      /* 114 */ 0,
      /* 115 */ 0,
      /* 116 */ 0,
      /* 117 */ 0,
      /* 118 */ 0,
      /* 119 */ 0,
      /* 120 */ 0,
      /* 121 */ 0,
      /* 122 */ 0,
      /* 123 */ 0,
      /* 124 */ 0,
      /* 125 */ 0,
      /* 126 */ 0,
      /* 127 */ 0,
      /* 128 */ 0,
      /* 129 */ 0,
      /* 130 */ 0,
      /* 131 */ 0,
      /* 132 */ 0,
      /* 133 */ 0,
      /* 134 */ 0,
      /* 135 */ 0,
      /* 136 */ 0,
      /* 137 */ 0,
      /* 138 */ 0,
      /* 139 */ 0,
      /* 140 */ 0,
      /* 141 */ 0,
      /* 142 */ 0,
      /* 143 */ 0,
      /* 144 */ 0,
      /* 145 */ 0,
      /* 146 */ 0,
      /* 147 */ 0,
      /* 148 */ 0,
      /* 149 */ 0,
      /* 150 */ 0,
      /* 151 */ 0,
      /* 152 */ 0,
      /* 153 */ 0,
      /* 154 */ 0,
      /* 155 */ 0,
      /* 156 */ 0,
      /* 157 */ 0,
      /* 158 */ 0,
      /* 159 */ 0,
      /* 160 */ 0,
      /* 161 */ 0,
      /* 162 */ 0,
      /* 163 */ 0,
      /* 164 */ 0,
      /* 165 */ 0,
      /* 166 */ 0,
      /* 167 */ 0,
      /* 168 */ 0,
      /* 169 */ 0,
      /* 170 */ 0,
      /* 171 */ 0,
      /* 172 */ 0,
      /* 173 */ 0,
      /* 174 */ 0,
      /* 175 */ 0,
      /* 176 */ 0,
      /* 177 */ 0,
      /* 178 */ 0,
      /* 179 */ 0,
      /* 180 */ 0,
      /* 181 */ 0,
      /* 182 */ 0,
      /* 183 */ 0,
      /* 184 */ 0,
      /* 185 */ 0,
      /* 186 */ 0,
      /* 187 */ 0,
      /* 188 */ 0,
      /* 189 */ 0,
      /* 190 */ 0,
      /* 191 */ 0,
      /* 192 */ 0,
      /* 193 */ 0,
      /* 194 */ 0,
      /* 195 */ 0,
      /* 196 */ 0,
      /* 197 */ 0,
      /* 198 */ 0,
      /* 199 */ 0,
      /* 200 */ 0,
      /* 201 */ 0,
      /* 202 */ 0,
      /* 203 */ 0,
      /* 204 */ 0,
      /* 205 */ 0,
      /* 206 */ 0,
      /* 207 */ 0,
      /* 208 */ 0,
      /* 209 */ 0,
      /* 210 */ 0,
      /* 211 */ 0,
      /* 212 */ 0,
      /* 213 */ 0,
      /* 214 */ 0,
      /* 215 */ 0,
      /* 216 */ 0,
      /* 217 */ 0,
      /* 218 */ 0,
      /* 219 */ 0,
      /* 220 */ 0,
      /* 221 */ 0,
      /* 222 */ 0,
      /* 223 */ 0,
      /* 224 */ 0,
      /* 225 */ 0,
      /* 226 */ 0,
      /* 227 */ 0,
      /* 228 */ 0,
      /* 229 */ 0,
      /* 230 */ 0,
      /* 231 */ 0,
      /* 232 */ 0,
      /* 233 */ 0,
      /* 234 */ 0,
      /* 235 */ 0,
      /* 236 */ 0,
      /* 237 */ 0,
      /* 238 */ 0,
      /* 239 */ 0,
      /* 240 */ 0,
      /* 241 */ 0,
      /* 242 */ 0,
      /* 243 */ 0,
      /* 244 */ 0,
      /* 245 */ 0,
      /* 246 */ 0,
      /* 247 */ 0,
      /* 248 */ 0,
      /* 249 */ 0,
      /* 250 */ 0,
      /* 251 */ 0,
      /* 252 */ 0,
      /* 253 */ 0,
      /* 254 */ 0,
      /* 255 */ 0
   };

#ifdef FPROTOTYPE
   void ChrInitialize(void)
#else
      void ChrInitialize()
#endif
      {
         static unsigned char dspdig[ ] = {  /* Contains the raw character set */
            'a','b','c','d','e','f','g','h','i','j','k','l' ,'m',
            'n','o','p','q','r','s','t','u','v','w','x','y' ,'z',
            '+','-','*','/','(',')','$','=',' ',',','.','\'','"',
            ':',';','\\','!','%','[',']'
         };
         int i;
         int chr;

         /*--------------------------------------------------------------------------
           ;
           ; Step 1: Initialize all character types and information at zero. Such
           ; charaters are all treated as "special" by the symbol processors.
           ;
           ;------------------------------------------------------------------------*/

         for(i = 0; i < 256; i++)
         {
            ChrTypeFlags[i] = ChrInformation[i] = 0;
            ChrDelimeters[i] = 0;
         }

         /*--------------------------------------------------------------------------
           ;
           ; Step 2: Flag all the uppercase letters as being letters and as being
           ; uppercase and lowercase letters as letters and lowercase. In addition
           ; record the code for the equivalent lower case letter in the character
           ; information vector for uppercase and store the char relative to one in
           ; the information vector for upper case.
           ;
           ;------------------------------------------------------------------------*/

         for(i = 0; i < 26; i++)
         {
            chr = ChrAlphabet[i];
            ChrTypeFlags[chr] = CLETTER | CUPPERC | CIDENTC;
            ChrInformation[chr] = dspdig[i];
            chr = dspdig[i];
            ChrTypeFlags[chr] = CLETTER | CLOWERC | CIDENTC;
            ChrInformation[chr] = (unsigned char) (i + 1);
         }
         /*--------------------------------------------------------------------------
           ;
           ; Step 3: Flag all numeric digits as numbers and record their values in
           ; the information vector.
           ;
           ;------------------------------------------------------------------------*/

         for(i = 0; i < 10; i++)
         {
            chr = ChrDigits[i];
            ChrTypeFlags[chr] = CNUMBER | CIDENTC;
            ChrInformation[chr] = (unsigned char)(i);
         }
         /*--------------------------------------------------------------------------
           ;
           ; Step 4: Flag all symbol characters as such and record their symbol
           ; type codes in the information vector.
           ;
           ;------------------------------------------------------------------------*/

         for(i = 26; i < 46; i++)
         {
            chr = dspdig[i];
            ChrTypeFlags[chr] = CSYMBOL;
            ChrInformation[chr] = (unsigned char) (i + 11);
         }
         /*--------------------------------------------------------------------------
           ;
           ; Step 5: Mark the underscore character as an identifier character and
           ; the various white space chararacters as white space.
           ;
           ;------------------------------------------------------------------------*/

         ChrTypeFlags[(unsigned char)'_'] = CIDENTC;
         ChrTypeFlags[(unsigned char)' '] |= CWSPACE;
         ChrTypeFlags[(unsigned char)'\t'] = CWSPACE;

         /*-------------------------------------------------------------------------
           ;
           ; Step 6: Mark the single and double quote characters as being QUOTEs.
           ;
           ;------------------------------------------------------------------------*/

         ChrTypeFlags[(unsigned char)'"'] |= CQUOTEC;
         ChrTypeFlags[(unsigned char)'\''] |= CQUOTEC;

         /*-------------------------------------------------------------------------
           ;
           ; Step 7: Initialize the null-character as belonging to all delimeter sets.
           ;
           ;-------------------------------------------------------------------------*/

         ChrDelimeters[0] = 255;
      }

   /*
     ;/hdr/ ************************************************************************
     ;
     ; Copyright 1986-1998 Promula Development Corporation ALL RIGHTS RESERVED
     ;
     ; name of module: FioOper.c -- Fortran I/O, Miscellaneous Operations
     ;
     ; global symbols defined:
     ;
     ; useful notes and assumptions:
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; edit 06/17/98 FKG: Changed names of global symbols to conform to the 
     ;                    Promula version 7.08 API naming conventions. 
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00
     ;
     ;/hdr/ ************************************************************************
   */
#define MEMOFUNC                  /* Access to memory allocation functions */
#define PRIOFUNC                  /* Gives access to PROMULA I/O functions */
#define OSYSFUNC                  /* Access to operating system functions */
#define RAWMFUNC                  /* Gives access to raw memory functions */
#define STRGFUNC                  /* Gives access to the string functions */
#define VARARGUS                  /* Gives access to variable arguments */
   // #include "platform.h"             /* Define the platform hosting this code */
   // #include "pfclib.h"               /* Define PROMULA.FORTRAN library symbols */
   // #include "chrapi.h"               /* Character manipulation Api */
   // #include "fncapi.h"               /* File naming conventins Api */
   /*
     ;/hdr/ ************************************************************************
     ;
     ; FncApi.h -- File Naming Conventions Api
     ;
     ; The interface defined in this header manages files -- their names and
     ; locations -- so that those files can have a consistent internal set of naming
     ; conventions which can be configured at runtime to point to actual files
     ; being maintained by the local platform in accordance with its own local
     ; conventions.
     ;
     ; global functions defined:
     ;
     ; int    FncAddToSearchList     Add to current search list
     ; int    FncBeginSearchList     Begin a new search list
     ; void   FncDestroySearchList   Destroy current search list
     ; char*  FncFindFile            Find a file using the current search list
     ;
     ; global parameters defined:
     ;
     ; int    FncDirLen              Maximum directory path length
     ; int    FncFileLen             Maximum complete file name length
     ;
     ; global structures defined: None
     ;
     ; global variables defined:
     ;
     ; char    FncDirChar            Path directory character
     ; char    FncSepChar            Multiple path separator character
     ; char    FncSysChar            Filesystem separation character
     ; char    FncXtnChar            Filename extension character
     ;
     ; useful notes and assumptions:
     ;
     ; The code within this module is crutial for obtaining platform portability.
     ; The applications level naming conventions must be independent of the
     ; particular host conventions.
     ;
     ;/hdr/ ************************************************************************
   */

#define FncDirLen     400         /* Maximum directory path length */
#define FncFileLen    500         /* Maximum complete file name length */
#define FncLocalLen   100         /* Maximum length of local file name */

#ifdef OSUNIX
#define FncDirChar   '/'          /* Path directory character */
#define FncSepChar   ':'          /* Multiple path separator character */
#define FncXtnChar   '.'          /* Filename extension separator */
#define FncSysChar   '\0'         /* File system separation character */
#endif
#ifdef OSVMS
#define FncDirChar   ']'          /* Path directory character */
#define FncSepChar   ';'          /* Multiple path separator character */
#define FncXtnChar   '.'          /* Filename extension separator */
#define FncSysChar   '\0'         /* File system separation character */
#endif
#ifdef OSMAC
#define FncDirChar   '\\'         /* Path directory character */
#define FncSepChar   ';'          /* Multiple path separator character */
#define FncXtnChar   '.'          /* Filename extension separator */
#define FncSysChar   '\0'         /* File system separation character */
#endif
#ifdef OSMSDOS
#define FncDirChar   '\\'         /* Path directory character */
#define FncSepChar   ';'          /* Multiple path separator character */
#define FncXtnChar   '.'          /* Filename extension separator */
#define FncSysChar   ':'          /* File system separation character */
#endif
#ifdef TSOPLAT
#define FncDirChar   '\\'         /* Path directory character */
#define FncSepChar   ';'          /* Multiple path separator character */
#define FncXtnChar   '.'          /* Filename extension separator */
#define FncSysChar   '\0'         /* File system separation character */
#endif
#ifdef DGEPLAT
#define FncDirChar   ':'          /* Path directory character */
#define FncSepChar   ','          /* Multiple path separator character */
#define FncXtnChar   '.'          /* Filename extension separator */
#define FncSysChar   '\0'         /* File system separation character */
#endif
#ifdef UCSPLAT
#define FncDirChar   '.'          /* Path directory character */
#define FncSepChar   ','          /* Multiple path separator character */
#define FncXtnChar   '/'          /* Filename extension separator */
#define FncSysChar   '\0'         /* File system separation character */
#endif

#ifdef FNC_DECLARE
   char* FncSearchList = NULL;       /* Points to current search list */
   int   FncSearchInitial = 1000;    /* Initial length of the search list area */
   int   FncSearchCurrent = 1000;    /* Current length of the search list area */
   int   FncSearchActual = 0;        /* Actual Length of search list */
#else
   extern char* FncSearchList;       /* Points to current search list */
   extern int   FncSearchInitial;    /* Initial length of the search list area */
   extern int   FncSearchCurrent;    /* Current length of the search list area */
   extern int   FncSearchActual;     /* Actual Length of search list */
#endif

#ifdef FPROTOTYPE
   int   FncAddToSearchList(char* Location,int Position);
   int   FncBeginSearchList(char* EnvName);
   void  FncDestroySearchList(void);
   int   FncFileName(char* path,char* name,int nname,char* ext,char* fullname);
   char* FncFindFile(char* FileName,char* LocalName);
   int   FncGetDir(char* Name,int Length);
   int   FncGetName(char* fileIdent,int nIdent,char* name,int iExt);
   int   FncMakeLocalFileName(char* FileName,char* LocalName,int LocalLength,
                              char* Extension,int XtnLength);
   int   FncParseFileName(char* FileName,int Length,int* LocalStart,int* XtnStart);
#else
   extern int   FncAddToSearchList();
   extern int   FncBeginSearchList();
   extern void  FncDestroySearchList();
   extern int   FncFileName();
   extern char* FncFindFile();
   extern int   FncGetDir();
   extern int   FncGetName();
   extern int   FncMakeLocalFileName();
   extern int   FncParseFileName();
#endif

#ifdef FPROTOTYPE
   // char* (*FioConvertName)(char* fname,int leng);
   // char* (*FioLocalName)(char* fname,int leng,int unit);
   // void  (*FioCloseOperation)(void); 
   int  fiogetname(char* lun,char* filename);
   int  fifindex(CONST char* s,int ns,CONST char* c,int nc);
   void fiortxt(void);
#else
   extern char* (*FioConvertName)();
   extern char* (*FioLocalName)();
   extern void  (*FioCloseOperation)(); 
   extern int  fiogetname();         /* Get name of file from OS */
   extern int  fifindex();           /* FORTRAN intrinsic function INDEX */
   extern void fiortxt();            /* Read next text record */
#endif

   extern const char**   comlin;           /* Pointer to command line arguments */
   extern FIOFILE* fiocurf;          /* Pointer to current FORTRAN file */
   extern int      fioblkn;          /* Blanks are null control flag */
   extern int      fioconv;          /* Runtime convention flags */
   extern char     fiocrec[FIOBMAX]; /* Current text record */
   extern int      fioerch;          /* Specifies whether error checking present */
   extern int      fioier;           /* Code for actual error encountered */
   extern int      fionunit;         /* Starting unit number for named files */
   extern int      fioplus;          /* Plus sign character */
   extern char     fioprefx[20];     /* Missing file name prefix character */
   extern LONG     fiorecl;          /* Desired record length */
   extern LONG     fiorecn;          /* Desired record number */
   extern FIOFILE  fioopnf[FIOMAX];  /* Describes all currently open files */
   extern int      fionopn;          /* Number of files currently open */
   extern LONG*    fiostat;          /* Returns an error code or zero */
   extern txtfile  fiosinp;          /* Current terminal input file */
   extern txtfile  fiosout;          /* Current terminal output file */
   extern int      ncomlin;          /* Number of command line arguments */

   static const char* runterr[ ] = 
      {
         /*1101 fiolun  */ "ELUN_EOF: end of file encountered",
         /*1102 fioopen */ "EOPN_EOF: end of file encountered",
         /*1103 fiorbiv */ "ERBV_EOF: end of file encountered",
         /*1104 fiortxt */ "ERTX_EOF: end of file",
         /*1105 fiowtxt */ "EWTX_EOF: write beyond end of file",
         /* 106 fiolun  */ "ELUN_RDO: write to readonly file",
         /* 107 fiointu */ "EINT_NAF: no active file structure",
         /* 108 fiolun  */ "ELUN_NAF: no active file structure",
         /* 109 fiointu */ "EINT_TMF: too many files open",
         /* 110 fiolun  */ "ELUN_TMF: too many files open",
         /* 111 fioname */ "ENAM_TMF: too many files open",
         /* 112 fiostio */ "ESIO_TMF: too many files open",
         /* 113 fioclose*/ "ECLO_PCF: physical close failure",
         /* 114 fioopen */ "EOPN_POF: physical open failed",
         /* 115 fioclose*/ "ECLO_POF: physical open failure",
         /* 116 fioback */ "EBCK_FRT: At front of file",
         /* 117 fioback */ "EBCK_DIR: direct access file",
         /* 118 fiolun  */ "ELUN_PWF: physical write failed",
         /* 119 fiorwbv */ "ERWV_PWF: physical write failure",
         /* 120 fiowbiv */ "EWBV_PWF: physical write error",
         /* 121 fiowef  */ "EWEF_PWF: physical write failure",
         /* 122 fiordb  */ "ERDB_IFS: invalid format specification",
         /* 123 fiordd  */ "ERDD_IFS: invalid format specification",
         /* 124 fiordf  */ "ERDF_IFS: invalid format specification",
         /* 125 fiordi  */ "ERDI_IFS: invalid format specification",
         /* 126 fiordl  */ "ERDL_IFS: invalid format specification",
         /* 127 fiords  */ "ERDS_IFS: invalid format specification",
         /* 128 fiordt  */ "ERDT_IFS: invalid format specification",
         /* 129 fiowrb  */ "EWRB_IFS: bad format specification",
         /* 130 fiowrs  */ "EWRS_IFS: bad format specification",
         /* 131 fiowrt  */ "EWRT_IFS: bad format specification",
         /* 132 fiowval */ "EWVL_IFS: bad format specification",
         /* 133 fionxtf */ "ENXF_EFS: bad of format control character",
         /* 134 fionxtf */ "ENXF_BTF: bad T format",
         /* 135 fionxtf */ "ENXF_BUS: bad B business format string",
         /* 136 fionxtf */ "ENXF_BBF: bad BN,Z format",
         /* 137 fioopen */ "EOPN_LNR: unit number out of range",
         /* 138 fionxtf */ "ENXF_DEL: mising terminating delimeter",
         /* 139 fionxtf */ "ENXF_HOL: bad holerith string",
         /* 140 fiorchk */ "ERCK_BUF: internal buffer exceeded",
         /* 141 fiordx  */ "ERDX_MLP: missing left parenthesis",
         /* 142 fiordx  */ "ERDX_COM: missing comma",
         /* 143 fiordx  */ "ERDX_MRP: missing right parenthesis",
         /* 144 fiordx  */ "ERDZ_MLP: missing left parenthesis",
         /* 145 fiordx  */ "ERDZ_COM: missing comma",
         /* 146 fiordx  */ "ERDZ_MRP: missing right parenthesis",
         /* 147 fiornl  */ "ERNL_MNI: missing namelist identifier",
         /* 148 fiornl  */ "ERNL_MVI: missing variable identifier",
         /* 149 fiornl  */ "ERNL_UVI: undefined variable identifier",
         /* 150 fiornl  */ "ERNL_SSV: subscripted scalar variable",
         /* 151 fiornl  */ "ERNL_NNS: nonnumeric subscripts",
         /* 152 fiornl  */ "ERNL_TMS: too many subscripts",
         /* 153 fiornl  */ "ERNL_EQL: missing equals sign",
         /* 154 fiornl  */ "ERNL_BSI: bad string input",
         /* 155 fiornl  */ "ERNL_MLP: complex missing left pren",
         /* 156 fiornl  */ "ERNL_COM: complex missing comma",
         /* 157 fiornl  */ "ERNL_MRP: complex missing right pren",
         /* 158 fiostod */ "ESTD_NNC: nonnumeric character in field",
         /* 159 fioopen */ "EOPN_BFZ: setting buffer size failed",
         /* 160 ftnxcons*/ "EXCN_ICB: illegal character in binary constant",
         /* 161 ftnxcons*/ "EXCN_ICO: illegal character in octal constant",
         /* 162 ftnxcons*/ "EXCN_ICH: illegal character in hex constant"
      };

   /*****************************************************************************/
   /*                                                                           */
   /* FIOERROR: Perform FORTRAN I/O Error Processing                            */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* If the FORTRAN I/O runtime system encounters an error, it sets an error   */
   /* code and calls this function. This function either sets an error return   */
   /* value or exits to the operating system with an error message. In the case */
   /* where an error code is returned to the calling function the parameter     */
   /* "clear" specifies whether or not the error processing control variables   */
   /* should be cleared prior to the return.                                    */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if there is no error flag set, else an error code. See the general */
   /* discussion if FORTRAN I/O capabilities for a listing of the possible      */
   /* error codes.                                                              */
   /*                                                                           */
   /* See also:                                                                 */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* See detailed steps below.                                                 */
   /*                                                                           */
   /*****************************************************************************/

#ifdef FPROTOTYPE
   int fioerror(int clear)
#else
      int fioerror(clear)
      int clear;                     /* Should error control be cleared? */
#endif
   {
      int numb;                 /* error number */
      int iret;                 /* Temporary storage for return value */
      const char* str;                /* Temporary string pointer */
      char  s[8];

      /**************************************************************************/
      /*                                                                        */
      /* Step 1: If a status variable is present, set it equal to the error     */
      /* code and branch to step 3. Note that an end-of-file error returns a    */
      /* -1; while all other errors simply return their code.                   */
      /*                                                                        */
      /**************************************************************************/

      if(!fioier) iret = 0;
      else if(fioier > 1000) iret = 1;
      else iret = 2;
      if(fiostat != NULL) 
      {
         if(fioier > 1000)  numb = -1;
         else numb = fioier;
#ifdef ISUNALIGNED
         if(fioerch < 10) put32s((char*)(fiostat),numb);
         else put16s((char*)(fiostat),(short)(numb));
#else
         if(fioerch < 10) *fiostat = numb;
         else *(short*) fiostat = (short)(numb);
#endif
         goto fioerr3;
      }

      /**************************************************************************/
      /*                                                                        */
      /* Step 2: If the calling logic is not doing error checking, and if there */
      /* is an error code set, write a message and end execution.               */
      /*                                                                        */
      /**************************************************************************/

      if(fioier == 0) goto fioerr3;
      numb = fioerch % 10;

      if(((numb & 1) == 0 && iret == 1) || ((numb & 2) == 0 && iret == 2)) 
      {
         numb = fioier % 1000;
         if(numb < 101 || numb > 159) 
         {
            wrtxtf(SCONSOL,"I/O error number ",17);
            ChrShortString(numb,s,0);
            str = s;
         } 
         else
         {
            wrtxtf(SCONSOL,"I/O error ",10);
            str = runterr[numb-101];
         }
         wrtxtf(SCONSOL,str,strlen(str));
         wrteol(SCONSOL);
         if(fiocurf != NULL) 
         {
            if(fiocurf->funit == INTERNAL)
               wrtxtf(SCONSOL,"On INTERNAL unit",16);
            else
            {
               wrtxtf(SCONSOL,"On unit ",8);
               ChrShortString(fiocurf->funit,s,0);
               wrtxtf(SCONSOL,s,strlen(s));
            }
            if(fiocurf->fname[0])
            {
               wrtxtf(SCONSOL," filename ",10);
               wrtxtf(SCONSOL,fiocurf->fname,strlen(fiocurf->fname));
            }
            wrteol(SCONSOL);
         }
         exit(numb);
      }

      /**************************************************************************/
      /*                                                                        */
      /* Step 3: If there is an error code set, return a one for an end-of-file */
      /* and a two for any other error; else return a zero. Before returning,   */
      /* clear the error control variables if requested to do so.               */
      /*                                                                        */
      /**************************************************************************/

   fioerr3:

      if(clear)
      {
         fioier = fioerch = 0;
         fiostat = NULL;
      }
      return iret;
   }

   /*****************************************************************************/
   /*                                                                           */
   /* FIONAME: Establish FORTRAN Unit by Name                                   */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* The FORTRAN INQUIRE statement allows the user to inquire about file       */
   /* status either via its logical unit number or via its name. If the name    */
   /* reference is being used, then this function is called. If there is no     */
   /* already existing structure for a unit with this name, then this function  */
   /* will attempt to create one.                                               */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if all went well, else an error code. See the general discussion   */
   /* of the FORTRAN I/O capabilities for a listing of the possible error codes.*/
   /*                                                                           */
   /* See also:                                                                 */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* See detailed steps below.                                                 */
   /*                                                                           */
   /*****************************************************************************/

#ifdef FPROTOTYPE
   int fioname(char* strg,int ns,int status)
#else
      int fioname(strg,ns,status)
      char* strg;                    /* Name of file */
   int ns;                        /* Number of characters in file name */
   int status;                    /* Status of reference: 1=inquire, 0=other */
#endif
   {
      int i;                    /* Dummy counter */
      int iret;                 /* Return value */
      // char* str;
      // int nstr;

      /**************************************************************************/
      /*                                                                        */
      /* Step 1: Determine if there currently is a FORTRAN file structure which */
      /* relates to this unit name. If the structure does exist, end local      */
      /* processing. NOTE that this check does not take possible aliases into   */
      /* account. The name specified must be identical to the one specified     */
      /* when it was opened.                                                    */
      /*                                                                        */
      /**************************************************************************/

      // str = strg;
      // nstr = ns;
      fioier = 0;
      while(ns && (*strg == ' '))
      {                              /* Strip any leading blanks */
         strg++;
         ns--;
      }
      while(ns && (*(strg+ns-1) == ' ')) ns--; /* Strip trailing blanks */
      for(i = iret = 0; i < fionopn; i++)
      {
         if(fioopnf[i].fstat & FUNUSED) continue;
         if((int)strlen(fioopnf[i].fname) != ns) continue;
         if(fifindex(strg,ns,fioopnf[i].fname,ns))
         {
            fiocurf = fioopnf+i;
            goto fioname9;
         }
      }

      /**************************************************************************/
      /*                                                                        */
      /* Step 2: There is no FORTRAN file structure which corresponds to the    */
      /* specified name. Find an empty structure and initialize it. If there is */
      /* no empty structure, set the error code accordingly and end local       */
      /* processing.                                                            */
      /*                                                                        */
      /**************************************************************************/

      fiocurf = NULL;
      if(status == 0)
      {
         fioier = ELUN_NAF;
         goto fioname9;
      }
      if(fionopn < FIOMAX) i = fionopn++;
      else for(i = 0; i < fionopn; i++)
         if(fioopnf[i].fstat & FUNUSED) break;
      if(i == fionopn)
      {
         fioier = ENAM_TMF;
         iret = fioerror(0);
         goto fioname9;
      }
      fiocurf = fioopnf+i;
      fiocurf->funit = -1;
      fiocurf->fstat = FUNUSED;
      fiocurf->fstat1 = 0;
      if(FioConvertName != NULL)
      {
         strg = FioConvertName(strg,ns);
         ns = strlen(strg);
      }
      cpymem(strg,fiocurf->fname,ns);
      fiocurf->fname[ns] = '\0';
      *(binfile*)&(fiocurf->ffile) = rdobinf(fiocurf->fname);
      if(binopner(*(binfile*)&(fiocurf->ffile)))
      {
         *(txtfile*)&(fiocurf->ffile) = nultxtf;
      }
      else {
         fiocurf->fstat |= FEXIST;
         clsbinf(*(binfile*)&(fiocurf->ffile));
      }
      fiocurf->fstat |= FNAMED;
      fiocurf->frsize = fiocurf->frecrd = 0;
   fioname9:
      return iret;
   }

   /*****************************************************************************/
   /*                                                                           */
   /* FIOSTIO: Establish FORTRAN Standard I/O                                   */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* Before any action can be performed on a standard I/O file, it must be     */
   /* associated with an existing FORTRAN file structure. If there is no        */
   /* already existing structure for the standard unit, then this function will */
   /* attempt to create one.                                                    */
   /*                                                                           */
   /* The type of standard unit to be used is determined by the action code as  */
   /* follows:                                                                  */
   /*                                                                           */
   /* Code  Standard unit                                                       */
   /* ----  -------------                                                       */
   /*   1   console(standard error)                                             */
   /*   2   standard input                                                      */
   /*   3   standard printer                                                    */
   /* other standard output                                                     */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if all went well, else an error code. See the general discussion   */
   /* of the FORTRAN I/O capabilities for a listing of the possible error codes.*/
   /*                                                                           */
   /* See also:                                                                 */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* See detailed steps below.                                                 */
   /*                                                                           */
   /*****************************************************************************/

#ifdef FPROTOTYPE
   int fiostio(int action)
#else
      int fiostio(action)
      int action;                    /* Action code for subsequent use */
#endif
   {
      int i;                    /* Dummy counter */
      int iret;                 /* Return value */
      txtfile unit;             /* Handle for desired unit */
      int iflag;                /* Special flags needed */

      /*------------------------------------------------------------------------*/
      /*                                                                        */
      /* Step 1: Determine if there currently is a FORTRAN file structure which */
      /* relates to this desired standard unit. If the structure does exist,    */
      /* branch to step 4;                                                      */
      /*                                                                        */
      /*------------------------------------------------------------------------*/

      fioier = 0;
      iflag = 0;
      if(fioerch >= 0)
      {
         fiostat = NULL;
         fioerch = 0;
      }
      else fioerch = -fioerch;
      if(action == 2) unit = SINPUT;
      else
      {
         if(action == 3) unit = SOUTPUT;
         else if(action == 1) unit = SCONSOL;
         else
         {
            if(fioconv & (ZDVAXFT | ZDFLEXF)) iflag = FTERMINAL;
            unit = SOUTPUT;
         }
         if(unit == nultxtf) unit = SOUTPUT;
         if(unit == SOUTPUT && fiosout != nultxtf) unit = fiosout;
      }
      for(i = iret = 0; i < fionopn; i++) 
         if(unit == *(txtfile*)&(fioopnf[i].ffile))
         {    
            fiocurf = fioopnf+i;
            goto fiostio4;
         }

      /*------------------------------------------------------------------------*/
      /*                                                                        */
      /* Step 2: There is no FORTRAN file structure which corresponds to the    */
      /* specified standard unit. Find an empty structure and initialize it. If */
      /* there is no empty structure, set the error code accordingly and end    */
      /* local processing.                                                      */
      /*                                                                        */
      /*------------------------------------------------------------------------*/

      fiocurf = NULL;
      if(fionopn < FIOMAX) i = fionopn++;
      else for(i = 0; i < fionopn; i++) if(fioopnf[i].fstat & FUNUSED) break;
      if(i == fionopn)
      {
         fioier = ESIO_TMF;
         iret = fioerror(0);
         goto fiostio9;
      }
      fiocurf = fioopnf+i;
      fiocurf->funit = -9999;
      fiocurf->fname[0] = '\0';
      *(txtfile*)&(fiocurf->ffile) = unit;
      fiocurf->frsize = fiocurf->frecrd = fiocurf->fstat = 0;
      fiocurf->fstat1 = iflag;
      if(fioconv & (ZDCARRI | ZDVAXFT)) fiocurf->fstat |= FCARRIG;
#ifdef BLANKNULL
      fiocurf->fstat |= FBLANK;
#endif

      /*------------------------------------------------------------------------*/
      /*                                                                        */
      /* Step 3: A file structure has been created for the unit. Set the        */
      /* remaining control switches.                                            */
      /*                                                                        */
      /*------------------------------------------------------------------------*/

      if(action == 2) fiocurf->fstat |= FEXIST;

      /*------------------------------------------------------------------------*/
      /*                                                                        */
      /* Step 4: An active FORTRAN file structure exists for the standard unit  */
      /* number. If the action is a coded read, read the next record and set    */
      /* the blanks are null control flag.                                      */
      /*                                                                        */
      /*------------------------------------------------------------------------*/

   fiostio4:
      if(action == 2)
      {
         fiortxt();
         fioblkn = fiocurf->fstat & FBLANK;
         if(!fioblkn) fioblkn = -1;
      }
   fiostio9:
      return iret;
   }

   /*****************************************************************************/
   /*                                                                           */
   /* FIOLREC: Position a FORTRAN File on a Record                              */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* This function positions the file associated the "current" FORTRAN file as */
   /* specified in the global variable "fiocurf" at the beginning of a          */
   /* specified record.                                                         */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if the positioning was successful, else an error code. See the     */
   /* general discussion if FORTRAN I/O capabilities for a listing of the       */
   /* possible error codes.                                                     */
   /*                                                                           */
   /* See also:                                                                 */
   /*                                                                           */
   /* Process:                                                                  */
   /* Use the standard C functions to reposition the file. If an error is       */
   /* encountered, do standard error processing.                                */
   /*                                                                           */
   /*****************************************************************************/

#ifdef FPROTOTYPE
   int fiolrec(LONG irec)
#else
      int fiolrec(irec)
      LONG irec;                     /* Record number desired */
#endif
   {
      LONG ioiad;

      if(!fioier && (fiocurf->fstat & FBINARY))
      {
         ioiad = (irec-1) * fiocurf->frsize;
         putbinf(*(binfile*)&(fiocurf->ffile),ioiad);
      }
      return fioier;
   }

   /*****************************************************************************/
   /*                                                                           */
   /* FIOUWL: Establish FORTRAN Unformatted Write Length                        */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* This function merely records the length of the following unformatted      */
   /* record to be written.                                                     */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* None                                                                      */
   /*                                                                           */
   /* See also:                                                                 */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* Simply record the desired record number.                                  */
   /*                                                                           */
   /*****************************************************************************/

#ifdef FPROTOTYPE
   void fiouwl(LONG* recl)
#else
      void fiouwl(recl)
      LONG* recl;                    /* Record number desired */
#endif
   {
      extern LONG fiorecl;           /* Desired record length */
#ifdef ISUNALIGNED
      fiorecl = get32s((char*)(recl));
#else
      fiorecl = *recl;
#endif
   }

   /*****************************************************************************/
   /*                                                                           */
   /* FIORWBV: FORTRAN Rewrite Binary Values                                    */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* This function rewrites binary values to a file.                           */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if all went well, else an error code. See the general discussion   */
   /* of the FORTRAN I/O capabilities for a listing of the possible error codes.*/
   /*                                                                           */
   /* See also:                                                                 */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* Simply rewrite the values.                                                */
   /*                                                                           */
   /*****************************************************************************/

#ifdef FPROTOTYPE
   int fiorwbv(VOID* value,int nvalue)
#else
      int fiorwbv(value,nvalue)
      VOID* value;                   /* Points to values being written */
   int nvalue;                    /* Number of bytes to be written */
#endif
   {
      if(!fioier)
      {
         if((int)wrbinf(*(binfile*)&(fiocurf->ffile),value,nvalue) < nvalue)
            fioier = ERWV_PWF;
      }
      return fioier;
   }

   /*****************************************************************************/
   /*                                                                           */
   /* FIOREC: Position a FORTRAN File on a Record                               */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* This function merely records a record number at which the FORTRAN file    */
   /* about to be accessed is to be positioned.                                 */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* None                                                                      */
   /*                                                                           */
   /* See also:                                                                 */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* Simply record the desired record number.                                  */
   /*                                                                           */
   /*****************************************************************************/

#ifdef FPROTOTYPE
   void fiorec(int irec)
#else
      void fiorec(irec)
      int irec;                      /* Record number desired */
#endif
   {
      fiorecn = irec;
   }
   /*****************************************************************************/
   /*                                                                           */
   /* FIOCLOSE: Close Current FORTRAN File                                      */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* This function closes the file associated the "current" FORTRAN file as    */
   /* specified in the global variable "fiocurf".                               */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if the close was successful, else an error code. See the general   */
   /* discussion if FORTRAN I/O capabilities for a listing of the possible      */
   /* error codes.                                                              */
   /*                                                                           */
   /* See also:                                                                 */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* Use the standard C function to close the file. If an error is             */
   /* encountered, do standard error processing.                                */
   /*                                                                           */
   /*****************************************************************************/

#ifdef FPROTOTYPE
   int fioclose(void)
#else
      int fioclose()
#endif
      {

         binfile xfile;            /* Ext file associated with internal file */
         int i;                    /* Dummy counter */
         int nc;                   /* Number of values in internal file */
         char* s;                  /* Pointer to start of file information */

         if(fiocurf->fstat & FINTERNAL) 
         {
            fiocurf->fstat |= FUNUSED;
            if((fiocurf->fname[0] != '\0') && 
               (!(fiocurf->fstat & FEXIST) || (fiocurf->fstat & FKEEP)))
            {
               xfile = inibinf(fiocurf->fname);
               if(binopner(xfile)) 
               {
                  fioier = ECLO_POF;
                  goto fioclose9;
               }
               if((s = fiocurf->fassob) == NULL) s = *(char**)&(fiocurf->ffile);
               nc = (int) (fiocurf->frecrd);
               wrbinf(xfile,s,nc);
               clsbinf(xfile);
            }
            goto fioclose9;
         }
         if(fiocurf->fstat1 & FBLKSIZE) free(fiocurf->fassob);
         if(fiocurf->fstat & FBINARY) 
         {
            if(clsbinf(*(binfile*)&(fiocurf->ffile))) fioier = ECLO_PCF;
         }
         else 
         {
            if(fiocurf->fstat1 & FNEEDCRLF) 
            {
               wrteol(*(txtfile*)&(fiocurf->ffile));
               fiocurf->fstat1 ^= FNEEDCRLF;
            }
            if(clstxtf(*(txtfile*)&(fiocurf->ffile))) fioier = ECLO_PCF;
         }
         if(fiocurf->fstat & FKEEP) 
         {
            fiocurf->fstat ^= FKEEP;
            fiocurf->fstat |= FNAMED;
            fiocurf->fstat |= FEXIST;
            if((fiocurf->fstat & FDIRECT) == 0) fiocurf->frsize = 0;
         }
         else 
         {

            /**************************************************************************/
            /*                                                                        */
            /* Note: When a file is physically deleted I must make certain that it is */
            /* not being "KEPT" under another unit number.                            */
            /*                                                                        */
            /**************************************************************************/

            fiocurf->fstat |= FUNUSED;
            fiocurf->fassov = NULL;
            if(((fiocurf->fstat & FDELETE) || (fiocurf->fstat & FSCRATCH)) && 
               (fiocurf->fname[0] != '\0'))  delfile(fiocurf->fname);
            s = fiocurf->fname;
            nc = strlen(s);
            for(i = 0; i < fionopn; i++)
            {
               if(fioopnf[i].fstat & FUNUSED) continue;
               if((int) strlen(fioopnf[i].fname) != nc) continue;
               if(fifindex(s,nc,fioopnf[i].fname,nc))
                  fioopnf[i].fstat |= FUNUSED;
            }
         }
      fioclose9:
         if(fioier == 0 && FioCloseOperation != NULL)
         {
            FioCloseOperation();
         }
         return fioerror(0);
      }
   /*****************************************************************************/
   /*                                                                           */
   /* FIOFINQU: Inquire About File Data                                         */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* This funtion is used to inquire about the various file data options       */
   /* associated with the current FORTRAN file structure. The particular data   */
   /* being inquired about is defined by the "option" parameter as follows:     */
   /*                                                                           */
   /* Option  Description of data                                               */
   /* ------  -------------------                                               */
   /*   1     Inquire about status of file                                      */
   /*   2     Inquire about existence of file                                   */
   /*   3     Inquire as to connection status of file                           */
   /*   4     Inquire as to files external unit number                          */
   /*   5     Inquire whether file has a name                                   */
   /*   6     Inquire for name of file                                          */
   /*   7     Inquire as to files access method                                 */
   /*   8     Inquire if file can be accessed sequentially                      */
   /*   9     Inquire if file can be directly accessed                          */
   /*  10     Inquire if file is formatted                                      */
   /*  11     Inquire if file can be opened as a text file                      */
   /*  12     Inquire if file can be opened as binary                           */
   /*  13     Inquire about file record length                                  */
   /*  14     Inquire about file next record number                             */
   /*  15     Inquire about file current blank convention                       */
   /*  16     Inquire about file carriage control convention                    */
   /*  17     Inquire about the record type                                     */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* None, this function is void.                                              */
   /*                                                                           */
   /* See also:                                                                 */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* Simply branch according to the type of data being set and do it. Illegal  */
   /* settings are simply ignored.                                              */
   /*                                                                           */
   /*****************************************************************************/

#ifdef FPROTOTYPE
   void fiofinqu(int option,char* str,int ns)
#else
      void fiofinqu(option,str,ns)
      int option;                    /* Specifies data being inquired about */
   char* str;                     /* Information to be returned */
   int ns;                        /* String length or integer information */
#endif
   {
      static const char* keyword[16] =
         {                              /* Return status keywords */
            "OLD",                      /* 0 */
            "NEW",                      /* 1 */
            "SCRATCH",                  /* 2 */
            "UNKNOWN",                  /* 3 */
            "SEQUENTIAL",               /* 4 */
            "DIRECT",                   /* 5 */
            "YES",                      /* 6 */
            "NO",                       /* 7 */
            "FORMATTED",                /* 8 */
            "UNFORMATTED",              /* 9 */
            "NULL",                     /* 10 */
            "ZERO",                     /* 11 */
            "FORTRAN",                  /* 12 */
            "NONE",                     /* 13 */
            "FIXED",                    /* 14 */
            "VARIABLE"                  /* 15 */
         };
      const char* key;                /* desired keyword to be returned */
      LONG len;                 /* A length */
      LONG ioiad;               /* Current file position */
      int irec;                 /* Current record number */

      switch(option)
      {
      case 1:                        /* Inquire about status of file */
         if(fiocurf->fstat & FNAMED) key = keyword[3];
         else if(fiocurf->fstat & FSCRATCH) key = keyword[2];
         else if(fiocurf->fstat & FEXIST) key = keyword[0];
         else key = keyword[1];
         goto fiofin2;
      case 2:                        /* Inquire about existence of file */
         if((fiocurf->fstat & FEXIST) || !(fiocurf->fstat & FNAMED)) len = 1;
         else len = 0;
         goto fiofin3;
      case 3:                        /* Inquire as to connection status of file */
         if(fiocurf->fstat & FNAMED) len = 0;
         else len = 1;
         goto fiofin3;
      case 4:                        /* Inquire as to files external unit number */
         len = fiocurf->funit;
         goto fiofin3;
      case 5:                        /* Inquire whether file has a name */
         if(fiocurf->fname[0]) len = 1;
         else len = 0;
         goto fiofin3;
      case 6:                        /* Inquire for name of file */
         if(FioLocalName != NULL)
            key = FioLocalName(fiocurf->fname,strlen(fiocurf->fname),
                               fiocurf->funit);
         else key = fiocurf->fname;
         goto fiofin2;
      case 7:                        /* Inquire as to files access method */
         if(fiocurf->fstat & FDIRECT) key = keyword[5];
         else key = keyword[4];
         goto fiofin2;
      case 8:                        /* Ask if file can be accessed sequentially */
         if(fiocurf->fstat & FDIRECT) key = keyword[7];
         else key = keyword[6];
         goto fiofin2;
      case 9:                        /* Ask if file can be directly accessed */
         if(fiocurf->fstat & FDIRECT) key = keyword[6];
         else key = keyword[7];
         goto fiofin2;
      case 10:                       /* Inquire if file is formatted */
         if(fiocurf->fstat & FBINARY) key = keyword[9];
         else key = keyword[8];
         goto fiofin2;
      case 11:                       /* Ask if file can be opened as text file */
         if(fiocurf->fstat & FBINARY) key = keyword[7];
         else key = keyword[6];
         goto fiofin2;
      case 12:                       /* Inquire if file can be opened as binary */
         if(fiocurf->fstat & FBINARY) key = keyword[6];
         else key = keyword[7];
         goto fiofin2;
      case 13:                       /* Inquire about file record length */
         len = fiocurf->frsize;
         if((fioconv & ZDVAXFT) && (fiocurf->fstat & FBINARY) && 
            (fiocurf->fstat & FDIRECT)) len /= sizeof(LONG);
         goto fiofin3;
      case 14:                       /* Ask about file current record number */
         if(fiocurf->fstat & FNAMED) len = 0;
         else
         {
            len = fiocurf->frsize;
            if(fiocurf->fstat & FINTERNAL) ioiad = fiocurf->frecrd;
            else if(fiocurf->fstat & FBINARY)
            {
               ioiad = posbinf(*(binfile*)&(fiocurf->ffile));
            }
            else 
            {
               len += EOLNCHAR;
               ioiad = postxtf(*(txtfile*)&(fiocurf->ffile));
            }
            irec =  (int)(ioiad / len + 1);
            if((ioiad % len) != 0) irec++;
            len = irec;
         }
         goto fiofin3;
      case 15:                       /* Ask about file current blank convention */
         if(fiocurf->fstat & FBLANK) key = keyword[10];
         else key = keyword[11];
         goto fiofin2;
      case 16:                       /* Inquire about file carriage control */
         if(fiocurf->fstat & FCARRIG) key = keyword[12];
         else key = keyword[13];
         goto fiofin2;
      case 17:                       /* Inquire about the record type */
         if(fiocurf->fstat & FDIRECT) key = keyword[14];
         else key = keyword[15];
         goto fiofin2;
      default:
         break;
      }
      goto fiofin9;

      /**************************************************************************/
      /*                                                                        */
      /* Step 2: A character string is being returned. Store as much of it as   */
      /* possible in the return character array, and if there is additional     */
      /* space, pad it with blanks.                                             */
      /*                                                                        */
      /**************************************************************************/

   fiofin2:
      irec = strlen(key);
      if(irec > ns) irec = ns;
      cpymem(key,str,ns);
      if(irec < ns) filmem(str+irec,ns-irec,' ');
      goto fiofin9;

      /**************************************************************************/
      /*                                                                        */
      /* Step 3: A long integer or logical response is being return.            */
      /*                                                                        */
      /**************************************************************************/

   fiofin3:
      *(LONG*)str = len;
   fiofin9:
      return;
   }

   /*****************************************************************************/
   /*                                                                           */
   /* FIOFVINQ: Inquire About File Value                                        */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* This function is used to inquire about the various file data options      */
   /* associated with the current FORTRAN file structure which return an        */
   /* integer or logical value. The particular value being inquired about is    */
   /* defined by the "option" parameter as follows:                             */
   /*                                                                           */
   /* Option  Description of data                                               */
   /* ------  -------------------                                               */
   /*   2     Inquire about existence of file                                   */
   /*   3     Inquire as to connection status of file                           */
   /*   4     Inquire as to files external unit number                          */
   /*   5     Inquire whether file has a name                                   */
   /*  13     Inquire about file record length                                  */
   /*  14     Inquire about file current record number                          */
   /*                                                                           */
   /* Note that this function is needed as distinct from the generic file       */
   /* information function because type conversions are required on the         */
   /* returned value.                                                           */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* The requested logical or integer value.                                   */
   /*                                                                           */
   /* See also:                                                                 */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* Simply obtain the desired value from the generic routine and return it.   */
   /*                                                                           */
   /*****************************************************************************/

#ifdef FPROTOTYPE
   LONG fiofvinq(int option)
#else
      LONG fiofvinq(option)
      int option;                    /* Specifies data being inquired about */
#endif
   {
      LONG value;               /* Desired value to be returned */

      value = 0;
      fiofinqu(option,(char*)&value,0);
      return value;
   }

   /*****************************************************************************/
   /*                                                                           */
   /* FIOREW: Rewind a FORTRAN File                                             */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* This function rewinds the file associated the "current" FORTRAN file as   */
   /* specified in the global variable "fiocurf".                               */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if the close was successful, else an error code. See the general   */
   /* discussion if FORTRAN I/O capabilities for a listing of the possible      */
   /* error codes.                                                              */
   /*                                                                           */
   /* See also:                                                                 */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* Use the standard C functions to reposition the file. If an error is       */
   /* encountered, do standard error processing.                                */
   /*                                                                           */
   /*****************************************************************************/

#ifdef FPROTOTYPE
   int fiorew(void)
#else
      int fiorew()
#endif
      {
         if(!fioier)
         {
            if(fiocurf->fstat1 & FREADLAST) fiocurf->fstat1 ^= FREADLAST;

            /*---------------------------------------------------------------------*/
            /*                                                                     */
            /* FKG: This code was added 05/15/97 to imitiate Prime Fortran and VAX */
            /* behavior which cause a file to be initialized by a rewind followed  */
            /* by a write. In the previous version this reinitialization would     */
            /* only occur if a READ was performed followed by a write. The setup   */
            /* for this logic is in fiolun.c                                       */
            /*                                                                     */
            /*---------------------------------------------------------------------*/

            if(!(fiocurf->fstat & FEXIST) && !(fiocurf->fstat & FDIRECT))
            {
               fiocurf->fstat |= FEXIST;
            }

            /*---------------------------------------------------------------------*/
            /*                                                                     */
            /* FKG: End of 05/15/97 change.                                        */
            /*                                                                     */
            /*---------------------------------------------------------------------*/

            if(fiocurf->fstat & FBINARY)
            {
               if(fiocurf->fstat & FINTERNAL) fiocurf->frecrd = 0;
               else rewbinf(*(binfile*)&(fiocurf->ffile));
            }
            else
            {
               if(fiocurf->fstat & FENDFIL) fiocurf->fstat ^= FENDFIL;
               if(fiocurf->fstat & FINTERNAL) fiocurf->frecrd = 0;
               else rewtxtf(*(txtfile*)&(fiocurf->ffile));
               if(!(fiocurf->fstat & FDIRECT)) fiocurf->frsize = 0;
            }
         }
         return fioerror(0);
      }

   /*****************************************************************************/
   /*                                                                           */
   /* FIOBACK: Backspace a FORTRAN File                                         */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* This function backspaces the file associated the "current" FORTRAN file   */
   /* as specified in the global variable "fiocurf". For files with fixed       */
   /* record lengths, this operation involves moving backwards in the file by   */
   /* the length of one record. For text files, this means moving backwards in  */
   /* the file to the position immediately following the second carriage-       */
   /* return-line-feed character pair which immediately preceed the current     */
   /* record. For other types of files, no backspace is possible. Note that if  */
   /* the file is currently positioned at its beginning, then no operation is   */
   /* performed.                                                                */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if the close was successful, else an error code. See the general   */
   /* discussion if FORTRAN I/O capabilities for a listing of the possible      */
   /* error codes.                                                              */
   /*                                                                           */
   /* See also:                                                                 */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* See detailed steps below.                                                 */
   /*                                                                           */
   /*****************************************************************************/

#ifdef FPROTOTYPE
   int fioback(void)
#else
      int fioback()
#endif
      {
         int irec;                 /* Desired record number */
         LONG length;              /* Current record length */
         LONG lpos;                /* Last file position */

         /**************************************************************************/
         /*                                                                        */
         /* Step 1: If the file is a direct access file, record its present        */
         /* position, back it up by the record length, and end local processing.   */
         /*                                                                        */
         /**************************************************************************/

         if(fiocurf->fstat1 & FREADLAST) fiocurf->fstat1 ^= FREADLAST;
         if(!(fiocurf->fstat & FDIRECT)) goto fioback2;
         if(fiocurf->fstat & FBINARY) 
         {
            lpos = posbinf(*(binfile*)&(fiocurf->ffile));
            lpos -= fiocurf->frsize; 
            if(lpos < 0) 
            {
               fioier = EBCK_FRT;
               goto fioback9;
            }
            putbinf(*(binfile*)&(fiocurf->ffile),lpos);
         }
         else 
         {
            lpos = postxtf(*(txtfile*)&(fiocurf->ffile));
            lpos -= (fiocurf->frsize + EOLNCHAR); 
            if(lpos < 0) 
            {
               fioier = EBCK_FRT;
               goto fioback9;
            }
            puttxtf(*(txtfile*)&(fiocurf->ffile),lpos);
         }
         goto fioback9;

         /**************************************************************************/
         /*                                                                        */
         /* Step 2: If the file is at a logical end-of-file, simply position it to */
         /* its current location. Then if the file is not a text file, branch to   */
         /* step 4.                                                                */
         /*                                                                        */
         /**************************************************************************/

      fioback2:
         if(fiocurf->fstat & FENDFIL) 
         {
            fiocurf->fstat ^= FENDFIL;
            lpos = postxtf(*(txtfile*)&(fiocurf->ffile));
            puttxtf(*(txtfile*)&(fiocurf->ffile),lpos);
            goto fioback9;
         }
         if(fiocurf->fstat & FBINARY) goto fioback3;

         /**************************************************************************/
         /*                                                                        */
         /* Step 3: The file is a text file. Obtain the current record number,     */
         /* rewind the file, and then read forward until the previous record is    */
         /* read.                                                                  */
         /*                                                                        */
         /**************************************************************************/

         if(!fiocurf->frsize) 
         {
            fioier = EBCK_FRT;
            goto fioback9;
         }
         irec = (int) (fiocurf->frsize - 1);
         fiorew();
         while(fiocurf->frsize < irec && fioier == 0) 
         {
            fiortxt();
         }
         goto fioback9;

         /**************************************************************************/
         /*                                                                        */
         /* Step 4: A binary file is being backspaced. Record the current file     */
         /* position, rewind the file, and then position forward using the record  */
         /* length values until the previous record is located.                    */
         /*                                                                        */
         /**************************************************************************/

      fioback3:
         lpos = posbinf(*(binfile*)&(fiocurf->ffile));
         if(lpos == 0) goto fioback9;
         lpos -= sizeof(LONG);
         putbinf(*(binfile*)&(fiocurf->ffile),lpos);
         rdbinf(*(binfile*)&(fiocurf->ffile),&length,sizeof(LONG));
         if(length >= 0) lpos -= (length + sizeof(LONG));
         putbinf(*(binfile*)&(fiocurf->ffile),lpos);
      fioback9:
         return fioerror(0);
      }

   /*****************************************************************************/
   /*                                                                           */
   /* FIOFLUSH: Flush a FORTRAN File                                            */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* This function flushes the file associated the "current" FORTRAN           */
   /* file as specified in the global variable "fiocurf".                       */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* This function always returns a zero, which indicates successful execution.*/
   /*                                                                           */
   /* See also:                                                                 */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* Use the standard C functions to flush the file.                           */
   /*                                                                           */
   /*****************************************************************************/

#ifdef FPROTOTYPE
   int fioflush(void)
#else
      int fioflush()
#endif
      {
         if(!fioier) 
         {
            if(fiocurf->fstat & FBINARY) 
            {
               fflush(*(binfile*)&(fiocurf->ffile));
            }
            else 
            {
               fflush(*(txtfile*)&(fiocurf->ffile));
            }
         }
         return fioerror(0);
      }

   /*****************************************************************************/
   /*                                                                           */
   /* FIOWEF: FORTRAN Write End-of-file                                         */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* This function writes an end-of-file to a file.                            */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if all went well, else an error code. See the general discussion   */
   /* of the FORTRAN I/O capabilities for a listing of the possible error codes.*/
   /*                                                                           */
   /* See also:                                                                 */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* Simply write the end-of-file.                                             */
   /*                                                                           */
   /*****************************************************************************/

#ifdef FPROTOTYPE
   int fiowef(void)
#else
      int fiowef()
#endif
      {
         char* intu;
         int ipos;

         if(!(fiocurf->fstat & FBINARY)) fiocurf->fstat |= FENDFIL;
         else if(!(fiocurf->fstat & FDIRECT))
         {
            fiocurf->frsize = -1;
            if(!(fiocurf->fstat & FINTERNAL))
            {
               if(wrbinf(*(binfile*)&(fiocurf->ffile),&(fiocurf->frsize),
                         sizeof(LONG)) < sizeof(LONG))  fioier = EWEF_PWF;
            }
            else
            {
               if((intu = fiocurf->fassob) == NULL)
                  intu = *(char**)&(fiocurf->ffile);
               ipos = (int) (fiocurf->frecrd);
               *(LONG*)(intu+ipos) = fiocurf->frsize;
               fiocurf->frecrd = ipos + sizeof(LONG);
            }
         }
         return fioier;
      }

   /*****************************************************************************/
   /*                                                                           */
   /* FIOGETFN: Get File Name for Current FORTRAN File                          */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* This function gets name for a file to be opened when the FORTRAN open     */
   /* request has not itself explicitly generated a file name. There are three  */
   /* possible sources of file names:                                           */
   /*                                                                           */
   /*    (1) If there was an Fname command line option specified, then the      */
   /*        file there named may contain information about the unit.           */
   /*                                                                           */
   /*    (2) When the fioprefx variable contains a formation rule, then the     */
   /*        name can be derived via that rule.                                 */
   /*                                                                           */
   /*    (3) Finally the user can be asked to supply the name himself.          */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* None, the function is void.                                               */
   /*                                                                           */
   /* See also:                                                                 */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* See detailed steps below                                                  */
   /*                                                                           */
   /*****************************************************************************/

#ifdef FPROTOTYPE
   void fiogetfn(void)
#else
      void fiogetfn()
#endif
      {
         static int havfname = 0;
         static char* finfo = NULL;
         binfile bin;
         int i;
         int nc;                   /* Length of file name */
         char s[8];
         int nfile;
         int ic;

         /**************************************************************************/
         /*                                                                        */
         /* Step 1: If we have not yet tried to locate a file description file, do */
         /* so now. If found, read it in.                                          */
         /*                                                                        */
         /**************************************************************************/

         if(havfname != 0) goto getfn2;
         havfname = 1;
         for(i = 1; i < ncomlin; i++)
         {
            if(*comlin[i] == 'F') break;
         }
         if(i >= ncomlin) goto getfn2;
         bin = opnbinf(comlin[i]+1);
         if(binopner(bin)) goto getfn2;
         sizbinf(bin);
         i = (int)(posbinf(bin));
         rewbinf(bin);
         finfo = (char*)(malloc(i));
         if(finfo != NULL) rdbinf(bin,finfo,i);
         clsbinf(bin);

         /**************************************************************************/
         /*                                                                        */
         /* Step 2: An attempt to locate a file description file has been made. If */
         /* no such file was found, branch to step 4.                              */
         /*                                                                        */
         /**************************************************************************/

      getfn2:
         if(finfo == NULL) goto getfn4;
         if(finfo[1] == 0) goto getfn4;

         /**************************************************************************/
         /*                                                                        */
         /* Step 3: File information is present. If a record exists for the        */
         /* current unit then establish the unit information and end local         */
         /* processing.                                                            */
         /*                                                                        */
         /**************************************************************************/

         nfile = finfo[1];
         ic = 2;
         while(nfile > 0)
         {
            if(fiocurf->funit == (int)(finfo[ic+1])) break;
            ic += (int)(finfo[ic]);
            nfile--;
         }
         if(nfile == 0) goto getfn4;
         nc = finfo[ic+6];
         cpymem(finfo+ic+7,fiocurf->fname,nc);
         fiocurf->fname[nc] = '\0';
         return;

         /**************************************************************************/
         /*                                                                        */
         /* Step 4: A file name must be constructed or obtained from the user.     */
         /* First convert the logical unit number to string form, and if under VAX */
         /* conventions make certain that it does not contain more that two digits.*/
         /*                                                                        */
         /**************************************************************************/

      getfn4:
         ChrShortString(fiocurf->funit,s,0);
         if(fiogetname(s,fiocurf->fname)) return;
         if(fioconv & ZDVAXFT)
         {
            if((int) strlen(s) > 2)
            {
               fioier = EOPN_LNR;
               return;
            }
         }

         /**************************************************************************/
         /*                                                                        */
         /* Step 5: If a filename formation rule is specified, then form its name  */
         /* based upon the unit number and end local processing.                   */
         /*                                                                        */
         /**************************************************************************/

         if(fioprefx[0])
         {
            ic = strlen(fioprefx);
            cpymem(fioprefx,fiocurf->fname,ic);
            nc = strlen(s);
            cpymem(s,fiocurf->fname+ic,nc+1);
            return;
         }

         /**************************************************************************/
         /*                                                                        */
         /* Step 6: The user must be asked to supply a name. Obtain it and end     */
         /* local processing.                                                      */
         /*                                                                        */
         /**************************************************************************/

         wrtxtf(SOUTPUT,"Filename missing for ",21);
         if(fiocurf->fstat & FEXIST) wrtxtf(SOUTPUT,"old",3);
         else if(fiocurf->fstat & FUNKNOWN) wrtxtf(SOUTPUT,"unknown",7);
         else wrtxtf(SOUTPUT,"new",3);
         wrtxtf(SOUTPUT," unit number ",13);
         wrtxtf(SOUTPUT,s,strlen(s));
         wrtxtf(SOUTPUT,"? ",2);
         rdtxtf(SINPUT,fiocurf->fname,40);
         nc = strlen(fiocurf->fname);
         while(nc > 0 && (fiocurf->fname[nc-1] == '\n' 
                          ||  fiocurf->fname[nc-1] == 10
                          ||  fiocurf->fname[nc-1] == 13)) nc--;
         fiocurf->fname[nc] = '\0';
         return;
      }

   /*****************************************************************************/
   /*                                                                           */
   /* FIOOPEN: Open Current FORTRAN File                                        */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* This function opens the file associated the "current" FORTRAN file as     */
   /* specified in the global variable "fiocurf".                               */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if the open went well, else an error code. See the general         */
   /* discussion of FORTRAN I/O capabilities for a listing of the possible      */
   /* error codes.                                                              */
   /*                                                                           */
   /* See also:                                                                 */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* See detailed steps below                                                  */
   /*                                                                           */
   /*****************************************************************************/

#ifdef FPROTOTYPE
   int fioopen(void)
#else
      int fioopen()
#endif
      {
         binfile bfile;
         int i;
         int nc;                   /* Length of file name */
         char* s;
         binfile xfile;            /* External file associated with internal file */
         LONG fiorecn;
         LONG fsize;               /* Current size of file */
         char LocalName[FIONMAX];
#ifdef VMSPLAT
         char str[8];
         char mrs[20];             /* Additional open parameter */
         mrs[0] = '\0';
#endif

         /**************************************************************************/
         /*                                                                        */
         /* Step 1: If the name of the file has not been assigned, obtain it from  */
         /* the user now via standard input and standard output.                   */
         /*                                                                        */
         /**************************************************************************/

         fiocurf->fuserinf = NULL;
         fiocurf->fuser = 0;
         if(fiocurf->fstat1 & FALREADYO)
         {
            fiocurf->fstat1 ^= FALREADYO;
            goto fioopen9;
         }
         if((fioconv & ZDVAXFT) && (fiocurf->fstat & FBINARY) && 
            (fiocurf->fstat & FDIRECT)) fiocurf->frsize *= sizeof(LONG);
         if(fiocurf->fstat & FINTERNAL)
         {
            fiocurf->frecrd = 0;
            fiocurf->fmaxrc = fiocurf->fbufsiz;
            if((s = fiocurf->fassob) == NULL) s = *(char**)&(fiocurf->ffile);
            *(LONG*)(s) = -1;
            if((fiocurf->fstat & (FEXIST | FUNKNOWN)) && (fiocurf->fname[0] != '\0'))
            {
               xfile = rdobinf(fiocurf->fname);
               if(binopner(xfile))
               {
                  if(fiocurf->fstat & FEXIST) fioier = EOPN_POF;
                  goto fioopen9;
               }
               nc = (int) fiocurf->fbufsiz;
               if((nc = rdbinf(xfile,s,nc)) < 1) fioier = EOPN_EOF;
               else fiocurf->fmaxrc = nc;
               clsbinf(xfile);
            }
            goto fioopen9;
         }
         if(!fiocurf->fname[0])
         {
            fiogetfn();
            if(fioier) goto fioopen9;
         }
         if(FncSearchList != NULL)
         {
            s = FncFindFile(fiocurf->fname,LocalName);
            if(s != NULL) cpymem(s,fiocurf->fname,strlen(s) + 1);
         }
         if(fiocurf->frsize)
         {
#ifdef VMSPLAT
            cpymem("mrs=",mrs,4);
            nc = ChrShortString(fiocurf->frsize,str,0);
            cpymem(str,mrs+4,nc+1);
#endif
            if(!(fiocurf->fstat & FDIRECT))
            {
               if((fioconv & ZDVAXFT) && !(fiocurf->fstat1 & FFIXEDTY))
                  fiocurf->frsize = 0;
               else
               {
                  fiocurf->fstat |= FDIRECT;
                  fiocurf->fstat1 |= FFIXEDFRM;
               }
            }
         }
         /**************************************************************************/
         /*                                                                        */
         /* Step 2: Attempt to open the file. Note that with "unknown" status if   */
         /* the "read" request fails, then a "write" request is made. If an error  */
         /* occurs during the open, perform standard error processing.             */
         /*                                                                        */
         /**************************************************************************/

         if((fiocurf->fstat & FBINARY) || ((fioconv & ZDVAXFT) && (fiocurf->fstat & FDIRECT)))
         {
#ifdef VMSPLAT
            if(fiocurf->fstat & FDIRECT)
            {
               if(fiocurf->fstat & FREADONLY)
                  *(binfile*)&(fiocurf->ffile) = 
                     us_fopen(fiocurf->fname,"rb","rfm=fix",
                           "shr=get,put,upd,upi",mrs);
               else if(fiocurf->fstat & (FEXIST | FUNKNOWN))
               {
                  *(binfile*)&(fiocurf->ffile) = 
                     us_fopen(fiocurf->fname,"r+b","rfm=fix",
                           "shr=get,put,upd,upi",mrs);
                  if(binopner(*(binfile*)&(fiocurf->ffile)))
                     *(binfile*)&(fiocurf->ffile) = 
                        us_fopen(fiocurf->fname,"rb","rfm=fix",
                              "shr=get,put,upd,upi",mrs);
               }
               else *(binfile*)&(fiocurf->ffile) = 
                       us_fopen(fiocurf->fname,"w+b","rfm=fix",
                             "shr=get,put,upd,upi",mrs);
               goto fioopen5;
            }
#endif
            if(fiocurf->fstat & FREADONLY)
            {
               *(binfile*)&(fiocurf->ffile) = rdobinf(fiocurf->fname);
            }
            else if(fiocurf->fstat & (FEXIST | FUNKNOWN))
            {
               bfile = opnbinf(fiocurf->fname);
               *(binfile*)&(fiocurf->ffile) = bfile;
               if(binopner(*(binfile*)&(fiocurf->ffile)))
               {
                  *(binfile*)&(fiocurf->ffile) = rdobinf(fiocurf->fname);
               }
            }
            else *(binfile*)&(fiocurf->ffile) = inibinf(fiocurf->fname);
#ifdef VMSPLAT
         fioopen5:
#endif
            if(binopner(*(binfile*)&(fiocurf->ffile)))
            {
               if(fiocurf->fstat & FUNKNOWN)
               {
                  *(binfile*)&(fiocurf->ffile) = inibinf(fiocurf->fname);
                  if(binopner(*(binfile*)&(fiocurf->ffile)))
                  {
                     fioier = EOPN_POF;
                     goto fioopen9;
                  }
               }
               else
               {
                  fioier = EOPN_POF;
                  goto fioopen9;
               }
            }
            if(fiocurf->fstat & FENDFIL)
            {
               sizbinf(*(binfile*)&(fiocurf->ffile));
               fiocurf->fstat ^= FENDFIL;
               if(fiocurf->fstat & FEXIST) fiocurf->fstat ^= FEXIST;
               if(fiocurf->fstat & FUNKNOWN) fiocurf->fstat ^= FUNKNOWN;
            }
            if((fiocurf->fstat & FDIRECT) && (fiocurf->fmaxrc > 0))
            {
               fiorecn = fiocurf->fmaxrc * fiocurf->frsize;
               sizbinf(*(binfile*)&(fiocurf->ffile));
               fsize = posbinf(*(binfile*)&(fiocurf->ffile));
               if(fsize < fiorecn)
               {
                  filmem(fiocrec,256,0);
                  putbinf(*(binfile*)&(fiocurf->ffile),fsize);
                  while(fiorecn > fsize)
                  {
                     if((fiorecn - fsize) > 256) i = 256;
                     else i = (int)(fiorecn - fsize);
                     wrbinf(*(binfile*)&(fiocurf->ffile),fiocrec,i);
                     fsize += i;
                  }
               }
               fiorecn = 0;
               putbinf(*(binfile*)&(fiocurf->ffile),fiorecn);
            }
         }
         else
         {
#ifdef VMSPLAT
            if(fiocurf->frsize != 0)
            {
               if(fiocurf->fstat & FREADONLY)
                  *(txtfile*)&(fiocurf->ffile) = 
                     us_fopen(fiocurf->fname,"r","rfm=fix",mrs);
               else if(fiocurf->fstat & (FEXIST | FUNKNOWN))
               {
                  *(txtfile*)&(fiocurf->ffile) = 
                     us_fopen(fiocurf->fname,"r+","rfm=fix",mrs);
                  if(txtopner(*(txtfile*)&(fiocurf->ffile)))
                     *(txtfile*)&(fiocurf->ffile) =
                        us_fopen(fiocurf->fname,"r","rfm=fix",mrs);
               }
               else *(txtfile*)&(fiocurf->ffile) =
                       us_fopen(fiocurf->fname,"w+"/*,"rfm=fix",mrs */);
               goto fioopen6;
            }
#endif
            if(fioconv & ZDVAXFT)
            {
               if(!cmpstrn(fiocurf->fname,"SYS$INPUT",9))
               { 
                  *(txtfile*)&(fiocurf->ffile) = SINPUT;
                  goto fioopen7;
               }
               if(!cmpstrn(fiocurf->fname,"SYS$OUTPUT",10))
               { 
                  *(txtfile*)&(fiocurf->ffile) = SOUTPUT;
                  if(fiocurf->fstat & FUNKNOWN) fiocurf->fstat ^= FUNKNOWN;
                  goto fioopen7;
               }
            }
            if(fiocurf->fstat & FREADONLY)
               *(txtfile*)&(fiocurf->ffile) = rdotxtf(fiocurf->fname);
            else if(fiocurf->fstat & (FEXIST | FUNKNOWN))
            {
               *(txtfile*)&(fiocurf->ffile) = opntxtf(fiocurf->fname);
               if(txtopner(*(txtfile*)&(fiocurf->ffile)))
                  *(txtfile*)&(fiocurf->ffile) = rdotxtf(fiocurf->fname);
            }
            else *(txtfile*)&(fiocurf->ffile) = initxtf(fiocurf->fname);
#ifdef VMSPLAT
         fioopen6:
#endif
            if(txtopner(*(txtfile*)&(fiocurf->ffile)))
            {
               if(fiocurf->fstat & FUNKNOWN)
               {
                  *(txtfile*)&(fiocurf->ffile) = initxtf(fiocurf->fname);
                  if(txtopner(*(txtfile*)&(fiocurf->ffile)))
                  {
                     fioier = EOPN_POF;
                     goto fioopen9;
                  }
               }
               else
               {
                  fioier = EOPN_POF;
                  goto fioopen9;
               }
            }
            if(fiocurf->fstat & FENDFIL)
            {
               siztxtf(*(txtfile*)&(fiocurf->ffile));
               fiocurf->fstat ^= FENDFIL;
               if(fiocurf->fstat & FEXIST) fiocurf->fstat ^= FEXIST;
               if(fiocurf->fstat & FUNKNOWN) fiocurf->fstat ^= FUNKNOWN;
            }
         }
#ifdef setbufsz
         if(fiocurf->fstat1 & FBLKSIZE)
         {
            nc = (int)(fiocurf->fbufsiz);
            if(setbufsz(*(txtfile*)&(fiocurf->ffile),fiocurf->fassob,nc) != 0)
            {
               fiocurf->fstat1 ^= FBLKSIZE;
               free(fiocurf->fassob);
               fioier = EOPN_BFZ;
            }
         }
#endif
      fioopen7:
         if(fiocurf->fstat & FUNKNOWN) fiocurf->fstat |= FEXIST;
         if(fiocurf->fassov != NULL) *(fiocurf->fassov) = 1;
      fioopen9:
         if(fioier) fiocurf->fstat |= FUNUSED;
         return fioerror(0);
      }
   /*****************************************************************************/
   /*                                                                           */
   /* FIOLUN: Establish FORTRAN Unit Number                                     */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* Before any action can be performed on a FORTRAN file, the logical unit    */
   /* number must be associated with an existing FORTRAN file structure. If     */
   /* there is no already existing structure for the unit number, then this     */
   /* function will attempt to create one. The form of this creation depends    */
   /* upon the type of the action to be performed. This action code is as       */
   /* follows:                                                                  */
   /*                                                                           */
   /* Code  Action to be performed                                              */
   /* ----  ----------------------                                              */
   /*   0   A file is to be opened with this logical unit number, if there is   */
   /*       an open structure currently associated with this number, then close */
   /*       the file.                                                           */
   /*   1   A structure for this logical unit number has been created. If it    */
   /*       cannot be found, then an error has occurred.                        */
   /*   2   A coded read is to be performed. If there is no structure defined,  */
   /*       then create one and open the file.                                  */
   /*   3   A binary read is to be performed. If there is no structure defined, */
   /*       then create one and open the file.                                  */
   /*   4   A coded write is to be performed. If there is no structure defined, */
   /*       then create one and create the file.                                */
   /*   5   A binary write is to be performed. If there is no structure defined,*/
   /*       then create one and create the file.                                */
   /*   6   A miscellaneous operation is to be performed. If there is no        */
   /*       structure defined, then create one and open the file.               */
   /*   7   A simple inquiry is being made                                      */
   /*   8   A find is to be performed                                           */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if all went well, else an error code. See the general discussion   */
   /* of the FORTRAN I/O capabilities for a listing of the possible error codes.*/
   /*                                                                           */
   /* See also:                                                                 */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* See detailed steps below.                                                 */
   /*                                                                           */
   /*****************************************************************************/

#ifdef FPROTOTYPE
   int fiolun                        /* Establish FORTRAN Unit Number */
      (
       int lun,                       /* Logical unit number of file */
       int action                     /* Action code for subsequent use */
       )
#else
      int fiolun(lun,action)

      int lun;                       /* Logical unit number of file */
   int action;                    /* Action code for subsequent use */

#endif
   {
      int i;                    /* Dummy counter */
      int iret;                 /* Return value */
      LONG fsize;               /* Current size of file */
      char* intu;               /* Offset of internal buffer */

      /**************************************************************************/
      /*                                                                        */
      /* Step 1: Determine if there currently is a FORTRAN file structure which */
      /* relates to this logical unit number. If the structure does exist,      */
      /* branch to step 4.                                                      */
      /*                                                                        */
      /**************************************************************************/

      fioier = 0;
      if(fioerch >= 0)
      {
         fiostat = NULL;
         fioerch = 0;
      }
      else fioerch = -fioerch;
      for(i = iret = 0; i < fionopn; i++)
      {
         if(lun == fioopnf[i].funit && !(fioopnf[i].fstat & FUNUSED))
         {    
            fiocurf = fioopnf+i;
            goto fiolun4;
         }
      }

      /**************************************************************************/
      /*                                                                        */
      /* Step 2: There is no FORTRAN file structure which corresponds to the    */
      /* specified logical unit number. Find an empty structure and initialize  */
      /* it. If there is no empty structure, set the error code accordingly and */
      /* end local processing.                                                  */
      /*                                                                        */
      /**************************************************************************/

      fiocurf = NULL;
      if(action == 1)
      {
         fioier = ELUN_NAF;
         iret = fioerror(0);
         goto fiolun9;
      }
      if(fionopn < FIOMAX) i = fionopn++;
      else for(i = 0; i < fionopn; i++) if(fioopnf[i].fstat & FUNUSED) break;
      if(i == fionopn)
      {
         fioier = ELUN_TMF;
         iret = fioerror(0);
         goto fiolun9;
      }
      fiocurf = fioopnf+i;
      fiocurf->funit = lun;
      fiocurf->fname[0] = '\0';
      *(txtfile*)&(fiocurf->ffile) = nultxtf;
      if(action == 7) fiocurf->fstat = FUNUSED;
      else fiocurf->fstat = 0;
      fiocurf->fstat1 = 0;
      fiocurf->frsize = fiocurf->frecrd = fiocurf->fmaxrc = 0;
      fiocurf->fassov = NULL;
#ifdef BLANKNULL
      fiocurf->fstat |= FBLANK;
#endif

      /**************************************************************************/
      /*                                                                        */
      /* Step 3: A file structure has been created for the unit. If this as an  */
      /* actual file action, then open the file in the appropriate manner.      */
      /*                                                                        */
      /**************************************************************************/

      if(action)
      {
         if(action == 7)
         {
            fiocurf->fstat |= FNAMED;
            fiocurf->fstat |= FEXIST;
            goto fiolun9;
         }
         if(action < 4 || action == 8) fiocurf->fstat |= FEXIST;
         else if(action == 6) fiocurf->fstat |= FUNKNOWN;
         if(action == 3 || action == 5)
         {
            fiocurf->fstat |= FBINARY;
            *(binfile*)&(fiocurf->ffile) = nulbinf;
         }
         fiocurf->fstat |= FENDFIL;
         fiocurf->fstat ^= FENDFIL;
         iret = fioopen();
      }
      else
      {
         fiocurf->fstat |= FUNKNOWN;
         goto fiolun9;
      }

      /**************************************************************************/
      /*                                                                        */
      /* Step 4: An active FORTRAN file structure exists for the logical unit   */
      /* number. If the action is to be an open, close the existing file and    */
      /* end local processing.                                                  */
      /*                                                                        */
      /**************************************************************************/

   fiolun4:
      if((action > 0) && (action != 7) && (fiocurf->fstat & FNAMED))
      {
         fiocurf->fstat ^= FNAMED;
         fiocurf->fstat |= FENDFIL;
         fiocurf->fstat ^= FENDFIL;
         iret = fioopen();
      }
      if(((action > 1 && action < 6) || (action == 8)) && 
         fiorecn != 0 && (fiocurf->fstat & FDIRECT))
      {
         if(fiorecn < 0)
         {
            fioback();
            goto fiolun5;
         }
         if(fiocurf->fassov != NULL) *(fiocurf->fassov) = fiorecn;
         if(fiocurf->fstat & FBINARY)
         {
            fiorecn = (fiorecn-1) * fiocurf->frsize;
            if(fiocurf->fstat & FINTERNAL) fiocurf->frecrd = fiorecn;
            else
            {
               sizbinf(*(binfile*)&(fiocurf->ffile));
               fsize = posbinf(*(binfile*)&(fiocurf->ffile));
               if(fsize < fiorecn)
               {
                  filmem(fiocrec,256,0);
                  putbinf(*(binfile*)&(fiocurf->ffile),fsize);
                  while(fiorecn > fsize)
                  {
                     if((fiorecn - fsize) > 256) i = 256;
                     else i = (int) (fiorecn - fsize);
                     wrbinf(*(binfile*)&(fiocurf->ffile),fiocrec,i);
                     fsize += i;
                  }
               }
               putbinf(*(binfile*)&(fiocurf->ffile),fiorecn);
               if(fiocurf->fstat1 & FREADLAST) fiocurf->fstat1 ^= FREADLAST;
            }
         }
         else
         {
            fiorecn = (fiorecn-1) * (fiocurf->frsize + EOLNCHAR);
            if(fiocurf->fstat & FINTERNAL) fiocurf->frecrd = fiorecn;
            else
            {
               if(fioconv & ZDVAXFT) putbinf(*(binfile*)&(fiocurf->ffile),fiorecn);
               else puttxtf(*(txtfile*)&(fiocurf->ffile),fiorecn);
               if(fiocurf->fstat1 & FREADLAST) fiocurf->fstat1 ^= FREADLAST;
            }
         }
      }
   fiolun5:
      fiorecn = 0;
      if(!action)
      {
         if(fiocurf->fstat & FEXIST) fiocurf->fstat ^= FEXIST;
         if(fiocurf->fstat & FNAMED) fiocurf->fstat ^= FNAMED;
         else
         {
            iret = 0;
            if(fiocurf->fname[0] != '\0') fiocurf->fstat1 |= FALREADYO;
            else fiocurf->fstat |= FUNKNOWN;
         }
      }

      /**************************************************************************/
      /*                                                                        */
      /* Step 5: If the action is a coded read, read the next record and set    */
      /* the blanks are null control flag and end local processing.             */
      /*                                                                        */
      /**************************************************************************/

      else if(action == 2)
      {
         if(fiocurf->fstat & FTERMIO)
         {
            if(fiosinp == nultxtf)
            {
               if(*(txtfile*)&(fiocurf->ffile) == SOUTPUT) fflush(SOUTPUT);
               *(txtfile*)&(fiocurf->ffile) = SINPUT;
            }
            else *(txtfile*)&(fiocurf->ffile) = fiosinp;
         }
         fiortxt();
         fioblkn = fiocurf->fstat & FBLANK;
         if(!fioblkn) fioblkn = -1;
      }

      /**************************************************************************/
      /*                                                                        */
      /* Step 6: If the action is a read from an unformatted sequential file,   */
      /* read the record length from the current position on the file and end   */
      /* local processing.                                                      */
      /*                                                                        */
      /**************************************************************************/

      else if((action == 3) && !(fiocurf->fstat & FDIRECT))
      {
         if(fiocurf->fstat & FINTERNAL)
         {
            fsize = fiocurf->frecrd;
            while(fsize % sizeof(LONG)) fsize++;
            intu = fiocurf->fassob;
            fiocurf->frsize = *(LONG*)(intu+fsize);
            if(fiocurf->frsize < 0) iret = fioier = ELUN_EOF;
            else fiocurf->frecrd = fsize + sizeof(LONG);
         }
         else
         {
            if(*(binfile*)&(fiocurf->ffile) == nulbinf)
            {
               iret = fioier = ELUN_EOF;
            }
            else
            {
               if(rdbinf(*(binfile*)&(fiocurf->ffile),&(fiocurf->frsize),
                         sizeof(LONG)) < sizeof(LONG))
               {
                  iret = fioier = ELUN_EOF;
                  fiocurf->fstat |= FENDFIL;
               }
               else if(fiocurf->frsize < 0) iret = fioier = ELUN_EOF;
            }
         }
      }

      /**************************************************************************/
      /*                                                                        */
      /* Step 7: If the action is a write to an unformatted sequential file,    */
      /* write a zero length to the file and record the current file position.  */
      /*                                                                        */
      /**************************************************************************/

      else if(action == 4)
      {
         if(fiocurf->fstat & FREADONLY)
         {
            fioier = ELUN_RDO;
            goto fiolun9;
         }
         if(fiocurf->fstat & FTERMIO)
         {
            if(fiosout == nultxtf) *(txtfile*)&(fiocurf->ffile) = SOUTPUT;
            else *(txtfile*)&(fiocurf->ffile) = fiosout;
         }
         fioplus = ' ';       
         ChrFlags |= CHR_PLUSSIGN;
         ChrFlags ^= CHR_PLUSSIGN;
      }
      else if(action == 5)
      {
         if(!(fiocurf->fstat & FINTERNAL)) fiocurf->frecrd = fiorecl;
         fiorecl = -1;
         if(fiocurf->fstat & FREADONLY)
         {
            fioier = ELUN_RDO;
            goto fiolun9;
         }
         if(fiocurf->fstat & FDIRECT) goto fiolun9;
         if(fiocurf->fstat & FEXIST)
         {
            fiocurf->fstat ^= FEXIST;
            if(fiocurf->fstat & FINTERNAL) fiocurf->frecrd = 0;
            else
            {
               clsbinf(*(binfile*)&(fiocurf->ffile));
               *(binfile*)&(fiocurf->ffile) = inibinf(fiocurf->fname);
            }
         }
         fiocurf->frsize = 0;
         fiocurf->fstat |= FBINARY;
         if(fiocurf->fstat & FINTERNAL)
         {
            fsize = fiocurf->frecrd;
            intu = fiocurf->fassob;
            while(fsize % sizeof(LONG)) fsize++;
            *(LONG*)(intu+fsize) = fiocurf->frsize;
            fiocurf->frsize = fiocurf->frecrd = fsize + sizeof(LONG);
            goto fiolun9;
         }
         if(fiocurf->fstat & FENDFIL)
         {
            fsize = posbinf(*(binfile*)&(fiocurf->ffile));
            putbinf(*(binfile*)&(fiocurf->ffile),fsize);
            fiocurf->fstat ^= FENDFIL;
         }
         if(fiocurf->frecrd < 0)
         {
            if(wrbinf(*(binfile*)&(fiocurf->ffile),&(fiocurf->frsize),
                      sizeof(LONG)) < sizeof(LONG)) iret = fioier = ELUN_PWF;
            else fiocurf->frsize = posbinf(*(binfile*)&(fiocurf->ffile));
         }
         else
         {
            if(wrbinf(*(binfile*)&(fiocurf->ffile),&(fiocurf->frecrd),
                      sizeof(LONG)) < sizeof(LONG)) iret = fioier = ELUN_PWF;
         }
      }
   fiolun9:
      return iret;
   }

   /*****************************************************************************/
   /*                                                                           */
   /* FTNLUN: Establish File for Logical Unit Number                            */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* Before a standard C I/O function can access a FORTRAN file, it must have  */
   /* access to the stream pointer associated with the logical unit number. The */
   /* logical unit number must be associated with an existing FORTRAN file      */
   /* structure. If there is no already existing structure for the unit number, */
   /* then this function will attempt to create one.                            */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* The stream pointer associated with the logical unit number.               */
   /*                                                                           */
   /* See also:                                                                 */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* Simply use fiolun to establish the structure and then return the text-    */
   /* file stream pointer from within that structure.                           */
   /*                                                                           */
   /*****************************************************************************/

#ifdef FPROTOTYPE
   txtfile ftnlun(int lun)
#else
      txtfile ftnlun(lun)
      int lun;                       /* Logical unit number of file */
#endif
   {
      fiolun(lun,4);
      if(fiocurf->fstat & FEXIST)
      {
         fiocurf->fstat ^= FEXIST;
         clstxtf(*(txtfile*)&(fiocurf->ffile));
         *(txtfile*)&(fiocurf->ffile) = initxtf(fiocurf->fname);
      }
      return *(txtfile*)&(fiocurf->ffile);
   }

   /*****************************************************************************/
   /*                                                                           */
   /* FIOINTU: Establish FORTRAN Internal Unit                                  */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* When some action is to be performed on an internal unit, typically a      */
   /* character storage area, this function initializes a file structure to     */
   /* point to this internal unit, and makes this file structure the current    */
   /* one. Note that if the record size is zero, then internal unit is a set of */
   /* pointers to C-style strings and not FORTRAN style string.                 */
   /*                                                                           */
   /* The form of the creation of the INTERNAL depends upon the type of the     */
   /* action to be performed. This action code is as follows:                   */
   /*                                                                           */
   /* Code  Action to be performed                                              */
   /* ----  ----------------------                                              */
   /*   0   An internal file is to be opened with the specified storage area    */
   /*       as its starting address. The size parameter specifies the overall   */
   /*       size of the area. If there is an open structure currently using     */
   /*       this starting address then it is simply rewound.                    */
   /*   1   A internal file with this starting address has been created. If it  */
   /*       cannot be found, then an error has occurred.                        */
   /*   2   A coded read is to be performed. If there is no structure defined,  */
   /*       then create one which will be removed at the end of the operation   */
   /*       (This is the standard behavior).                                    */
   /*   3   A binary read is to be performed. If there is no structure defined, */
   /*       then create one which will be removed at the end of the operation.  */
   /*   4   A coded write is to be performed. If there is no structure defined, */
   /*       then create one which will be removed at the end of the operation   */
   /*       (This is the standard behavior).                                    */
   /*   5   A binary write is to be performed. If there is no structure         */
   /*       defined, then create one which will be removed at the end of the    */
   /*       operation.                                                          */
   /*   6   A miscellaneous operation is to be performed. If there is no        */
   /*       structure defined, then an error has occurred.                      */
   /*   7   A simple inquiry is being made. If there is no structure defined    */
   /*       defined, then create one which will be removed at the end of the    */
   /*       operation.                                                          */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if all went well, else an error code. See the general discussion   */
   /* of the FORTRAN I/O capabilities for a listing of the possible error codes.*/
   /*                                                                           */
   /* See also:                                                                 */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* See detailed steps below.                                                 */
   /*                                                                           */
   /*****************************************************************************/

#ifdef FPROTOTYPE
   int fiointu(char* intu,int rsize,int action)
#else
      int fiointu(intu,rsize,action)
      char* intu;                    /* Pointer to internal storage */
   int rsize;                     /* Record size */
   int action;                    /* Specifies action to be performed */
#endif
   {
      int i;                    /* Dummy counter */
      int iret;                 /* Return value */
      LONG fsize;               /* Current size of file */

      /**************************************************************************/
      /*                                                                        */
      /* Step 1: Determine if there currently is an INTERNAL FORTRAN file       */
      /* structure which has the specified address as its start. If the         */
      /* structure does exist, branch to step 4.                                */
      /*                                                                        */
      /**************************************************************************/

      fioier = iret = 0;
      if(fioerch >= 0)
      {
         fiostat = NULL;
         fioerch = 0;
      }
      else fioerch = -fioerch;
      for(i = 0; i < fionopn; i++)
      {
         if((fioopnf[i].funit == INTERNAL) && !(fioopnf[i].fstat & FUNUSED) &&
            (*(char**)&(fioopnf[i].ffile) == intu))
         {
            fiocurf = fioopnf+i;
            goto fiointu4;
         }
      }

      /**************************************************************************/
      /*                                                                        */
      /* Step 2: There is no FORTRAN file structure which corresponds to the    */
      /* specified logical unit number. Find an empty structure and initialize  */
      /* it. If there is no empty structure, set the error code accordingly and */
      /* end local processing.                                                  */
      /*                                                                        */
      /**************************************************************************/

      fiocurf = NULL;
      if(action == 1 || action == 6)
      {
         fioier = EINT_NAF;
         iret = fioerror(0);
         goto fiointu9;
      }
      if(fionopn < FIOMAX) i = fionopn++;
      else for(i = 0; i < fionopn; i++) if(fioopnf[i].fstat & FUNUSED) break;
      if(i == fionopn)
      {
         fioier = EINT_TMF;
         iret = fioerror(0);
         goto fiointu9;
      }
      fiocurf = fioopnf+i;
      fiocurf->fname[0] = '\0';
      *(char**)&(fiocurf->ffile) = intu;
      fiocurf->fassob = NULL;
      fiocurf->fbufsiz = fiocurf->fmaxrc = rsize;
      fiocurf->frsize = fiocurf->frecrd = 0;
      fiocurf->fstat = FINTERNAL;
      fiocurf->fstat1 = 0;
      if(action == 0) fiocurf->funit = INTERNAL;
      else
      {
         fiocurf->funit = -1;
         fiocurf->fstat |= FUNUSED;
         if(action != 7) fiocurf->frsize = rsize;
      }
#ifdef BLANKNULL
      fiocurf->fstat |= FBLANK;
#endif

      /**************************************************************************/
      /*                                                                        */
      /* Step 3: A file structure has been created for the unit. If this as an  */
      /* actual file action, then initialize the structure in the appropriate   */
      /* manner.                                                                */
      /*                                                                        */
      /**************************************************************************/

      if(action)
      {
         if(action == 7)
         {
            fiocurf->fstat |= FNAMED;
            fiocurf->fstat |= FEXIST;
            goto fiointu9;
         }
         if(action < 4) fiocurf->fstat |= FEXIST;
         else if(action == 6) fiocurf->fstat |= FUNKNOWN;
         if(action == 3 || action == 5) fiocurf->fstat |= FBINARY;
         fiocurf->fstat |= FENDFIL;
         fiocurf->fstat ^= FENDFIL;
      }
      else
      {
         fiocurf->fstat |= FUNKNOWN;
         goto fiointu9;
      }

      /**************************************************************************/
      /*                                                                        */
      /* Step 4: An active FORTRAN file structure exists for the memory buffer. */
      /* Perform any needed operations and end local processing.                */
      /*                                                                        */
      /**************************************************************************/

   fiointu4:
      if((action > 0) && (action < 7) && (fiocurf->fstat & FNAMED))
      {
         fiocurf->fstat ^= FNAMED;
         fiocurf->fstat |= FENDFIL;
         fiocurf->fstat ^= FENDFIL;
         fiocurf->frecrd = 0;
      }
      if(action > 1 && action < 6 && fiorecn > 0 && (fiocurf->fstat & FDIRECT))
      {
         if(fiocurf->fstat & FBINARY)
         {
            fiorecn = (fiorecn-1) * fiocurf->frsize;
            fiocurf->frecrd = fiorecn;
         }
         else 
         {
            fiorecn = (fiorecn-1) * (fiocurf->frsize + 1);
            fiocurf->frecrd = fiorecn;
         }
      }
      fiorecn = 0;

      /**************************************************************************/
      /*                                                                        */
      /* Step 5: If the action is a coded read, read the next record and set    */
      /* the blanks are null control flag and end local processing.             */
      /*                                                                        */
      /**************************************************************************/

      if(action == 2) {
         fiortxt();
         fioblkn = fiocurf->fstat & FBLANK;
         if(!fioblkn) fioblkn = -1;
      }

      /**************************************************************************/
      /*                                                                        */
      /* Step 6: If the action is a read from an unformatted sequential file,   */
      /* get the record length from the current position on the file and end    */
      /* local processing.                                                      */
      /*                                                                        */
      /**************************************************************************/

      else if((action == 3) && !(fiocurf->fstat & FDIRECT))
      {
         fsize = fiocurf->frecrd;
         while(fsize % sizeof(LONG)) fsize++;
         fiocurf->frsize = *(LONG*)(intu+fsize);
         fiocurf->frecrd = fsize + sizeof(LONG);
      }

      /**************************************************************************/
      /*                                                                        */
      /* Step 7: If the action is a write to an unformatted sequential file,    */
      /* write a zero length to the file and record the current file position.  */
      /*                                                                        */
      /**************************************************************************/

      else if(action == 4)
      {
         fioplus = ' ';       
         ChrFlags |= CHR_PLUSSIGN;
         ChrFlags ^= CHR_PLUSSIGN;
      }
      else if(action == 5)
      {
         if(fiocurf->fstat & FDIRECT) goto fiointu9;
         fiorecl = -1;
         if(fiocurf->fstat & FEXIST)
         {
            fiocurf->frecrd = 0;
            fiocurf->fstat ^= FEXIST;
         }
         fiocurf->frsize = 0;
         fiocurf->fstat |= FBINARY;
         fsize = fiocurf->frecrd;
         while(fsize % sizeof(LONG)) fsize++;
         *(LONG*)(intu+fsize) = fiocurf->frsize;
         fiocurf->frsize = fiocurf->frecrd = fsize + sizeof(LONG);
      }
   fiointu9:
      return iret;
   }

   /*****************************************************************************/
   /*                                                                           */
   /* FIOSTATUS: Set FORTRAN I/O Error Status                                   */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* If the FORTRAN I/O runtime system encounters an error, it sets an error   */
   /* code and calls function "fioerr". The behavior of that function depends   */
   /* upon how the code using the I/O system is doing error processing. This    */
   /* function establishes the error code return variable and the error         */
   /* checking level. an error return value or exits to the operating system    */
   /* with an                                                                   */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* None, the function is void.                                               */
   /*                                                                           */
   /* See also:                                                                 */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* Simply set the global variables "fioerch" and "fiostat" to the user       */
   /* specified values and initialize the status variable at zero.              */
   /*                                                                           */
   /*****************************************************************************/

#ifdef FPROTOTYPE
   void fiostatus(LONG* iostat,int error)
#else
      void fiostatus(iostat,error)
      LONG* iostat;                  /* Address of error status variable */
   int error;                     /* Error testing switch */
#endif
   {
      int izero;
      short szero;

      fioerch = -error;
      fiostat = (LONG*)(iostat);
      if(fiostat != NULL)
      {
         if(error < 10)
         {
            izero = 0;
#ifdef ISUNALIGNED
            put32s((char*)(fiostat),izero);
#else
            *fiostat = izero;
#endif
         }
         else
         {
            szero = 0;
#ifdef ISUNALIGNED
            put16s((char*)(fiostat),szero);
#else
            *(short*) fiostat = szero;
#endif
         }
      }
   }

   /*****************************************************************************/
   /*                                                                           */
   /* FTNCLOSE: FORTRAN Close Statement                                         */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* Executes the FORTRAN CLOSE statement when translated via a non-optimized  */
   /* user bias. In addition to the logical unit number, this function takes a  */
   /* variable number of parameters which specify the actual data to be         */
   /* supplied to the close operation. The parameter type codes passed to this  */
   /* function are as follows:                                                  */
   /*                                                                           */
   /* Code   Parameter  Description of Use                                      */
   /* ----   ---------  ------------------                                      */
   /*  0     ----       Ends the list of parameters                             */
   /*  1     LONG*      Points to an error return variable                      */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if the close went well, else an error code. See the general        */
   /* discussion of FORTRAN I/O capabilities for a listing of the possible      */
   /* error codes.                                                              */
   /*                                                                           */
   /* See also:                                                                 */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* Initialize the I/O system via the logical unit number, pass the close     */
   /* argument information, finally do the physical close.                      */
   /*                                                                           */
   /*****************************************************************************/

#ifdef VARUNIXV
   int ftnclose(va_alist) va_dcl     /* FORTRAN Close Statement */
#else
      int ftnclose(int lun,...)
#endif
      {
         va_list argptr;
         int oper;
         LONG* iostat;
         int ier;
         char* str;
         int ipass;
         char* ifile;              /* Pointer to internal file storage */
         int nifile;               /* Internal file record length */

#ifdef VARUNIXV
         int lun;
         va_start(argptr);
         lun = va_arg(argptr,int);
#else
         va_start(argptr,lun);
#endif
         nifile = 0;
         ifile = NULL;
         if(lun == NAMED)
         {
            ifile = va_arg(argptr,char*);
            nifile = va_arg(argptr,int);
            fioname(ifile,nifile,0);
            if(fioier == 0) lun = fiocurf->funit;
         }
         if(lun == INTERNAL)            /* This is an internal file */
         {
            ifile = va_arg(argptr,char*);
            nifile = va_arg(argptr,int);
         }
         ier = ipass = 0;
         for(;;)
         {
            oper = va_arg(argptr,int);
            if(oper == 1)
            {
               iostat = va_arg(argptr,LONG*);
               ier = va_arg(argptr,int);
               fiostatus(iostat,ier);
               if(fioier != 0)
               {
                  va_end(argptr);
                  return fioerror(1);
               }
            }
            else
            {
               if(!ipass)
               {
                  if(lun == INTERNAL) fiointu(ifile,nifile,6);
                  else fiolun(lun,6);
                  ipass = 1;
               }
               if(oper == 3)
               {
                  str = va_arg(argptr,char*);
                  fiofdata(2,str,1);
               }
               else
               {
                  va_end(argptr);
                  fioclose();
                  return fioerror(1);
               }
            }
         }
      }
   /*****************************************************************************/
   /*                                                                           */
   /* FTNBACK: FORTRAN Backspace Statement                                      */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* Executes the FORTRAN BACKSPACE statement when translated via a non-       */
   /* optimized user bias. In addition to the logical unit number, this         */
   /* function takes a variable number of parameters which specify the actual   */
   /* data to be supplied to the backspace operation. The parameter type codes  */
   /* passed to this function are as follows:                                   */
   /*                                                                           */
   /* Code   Parameter  Description of Use                                      */
   /* ----   ---------  ------------------                                      */
   /*  0     ----       Ends the list of parameters                             */
   /*  1     LONG*      Points to an error return variable                      */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if the backspace went well, else an error code. See the general    */
   /* discussion of FORTRAN I/O capabilities for a listing of the possible      */
   /* error codes.                                                              */
   /*                                                                           */
   /* See also:                                                                 */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* Initialize the I/O system via the logical unit number, pass the rewind    */
   /* argument information, finally do the physical backspace.                  */
   /*                                                                           */
   /*****************************************************************************/

#ifdef VARUNIXV
   int ftnback(va_alist) va_dcl      /* FORTRAN Backspace Statement */
#else
      int ftnback(int lun,...)
#endif
      {
         va_list argptr;
         int oper;
         LONG* iostat;
         int ier;
         char* ifile;              /* Pointer to internal file storage */
         int nifile;               /* Internal file record length */

#ifdef VARUNIXV
         int lun;
         va_start(argptr);
         lun = va_arg(argptr,int);
#else
         va_start(argptr,lun);
#endif
         nifile = 0;
         ifile = NULL;
         if(lun == INTERNAL)            /* This is an internal file */
         {
            ifile = va_arg(argptr,char*);
            nifile = va_arg(argptr,int);
         }
         ier = 0;
         for(;;)
         {
            oper = va_arg(argptr,int);
            if(oper == 1)
            {
               iostat = va_arg(argptr,LONG*);
               ier = va_arg(argptr,int);
               fiostatus(iostat,ier);
            }
            else
            {
               if(lun == INTERNAL) fiointu(ifile,nifile,6);
               else fiolun(lun,6);
               va_end(argptr);
               fioback();
               return fioerror(1);
            }
         }
      }

   /*****************************************************************************/
   /*                                                                           */
   /* FTNREW: FORTRAN Rewind Statement                                          */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* Executes the FORTRAN REWIND statement when translated via a non-optimized */
   /* user bias. In addition to the logical unit number, this function takes a  */
   /* variable number of parameters which specify the actual data to be         */
   /* supplied to the rewind function. The parameter type codes passed to this  */
   /* function are as follows:                                                  */
   /*                                                                           */
   /* Code   Parameter  Description of Use                                      */
   /* ----   ---------  ------------------                                      */
   /*  0     ----       Ends the list of parameters                             */
   /*  1     LONG*      Points to an error return variable                      */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if the rewind went well, else an error code. See the general       */
   /* discussion of FORTRAN I/O capabilities for a listing of the possible      */
   /* error codes.                                                              */
   /*                                                                           */
   /* See also:                                                                 */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* Initialize the I/O system via the logical unit number, pass the rewind    */
   /* argument information, finally do the physical rewind.                     */
   /*                                                                           */
   /*****************************************************************************/

#ifdef VARUNIXV
   int ftnrew(va_alist) va_dcl       /* FORTRAN Rewind Statement */
#else
      int ftnrew(int lun,...)
#endif
      {
         va_list argptr;
         int oper;
         LONG* iostat;
         int ier;
         char* ifile;              /* Pointer to internal file storage */
         int nifile;               /* Internal file record length */

#ifdef VARUNIXV
         int lun;
         va_start(argptr);
         lun = va_arg(argptr,int);
#else
         va_start(argptr,lun);
#endif
         nifile = 0;
         ifile = NULL;
         if(lun == INTERNAL)            /* This is an internal file */
         {
            ifile = va_arg(argptr,char*);
            nifile = va_arg(argptr,int);
         }
         ier = 0;
         for(;;)
         {
            oper = va_arg(argptr,int);
            if(oper == 1)
            {
               iostat = va_arg(argptr,LONG*);
               ier = va_arg(argptr,int);
               fiostatus(iostat,ier);
            }
            else
            {
               if(lun == INTERNAL) fiointu(ifile,nifile,6);
               else fiolun(lun,6);
               va_end(argptr);
               fiorew();
               return fioerror(1);
            }
         }
      }

   /*****************************************************************************/
   /*                                                                           */
   /* FTNFLUSH: FORTRAN Flush Statement                                         */
   /*                                                                           */
   /* Copyright 1988-94 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Synopsis:                                                                 */
   /*                                                                           */
   /* Description:                                                              */
   /*                                                                           */
   /* Executes the FORTRAN FLUSH statement when translated via a non-optimized  */
   /* user bias. In addition to the logical unit number, this function takes a  */
   /* variable number of parameters which specify the actual data to be         */
   /* supplied to the flush function. The parameter type codes passed to this   */
   /* function are as follows:                                                  */
   /*                                                                           */
   /* Code   Parameter  Description of Use                                      */
   /* ----   ---------  ------------------                                      */
   /*  0     ----       Ends the list of parameters                             */
   /*  1     LONG*      Points to an error return variable                      */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if the flush went well, else an error code. See the general        */
   /* discussion of FORTRAN I/O capabilities for a listing of the possible      */
   /* error codes.                                                              */
   /*                                                                           */
   /* See also:                                                                 */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* Initialize the I/O system via the logical unit number, pass the           */
   /* flush argument information, finally do the physical flush.                */
   /*                                                                           */
   /*****************************************************************************/

#ifdef VARUNIXV
   int ftnflush(va_alist)  va_dcl
#else
      int ftnflush(int lun,...)
#endif
      {
         va_list argptr;
         int oper;
         LONG* iostat;
         int ier;
         char* ifile;              /* Pointer to internal file storage */
         int nifile;               /* Internal file record length */

#ifdef VARUNIXV
         int lun;
         va_start(argptr);
         lun = va_arg(argptr,int);
#else
         va_start(argptr,lun);
#endif
         nifile = 0;
         ifile = NULL;
         if(lun == INTERNAL) 
         {                              /* This is an internal file */
            ifile = va_arg(argptr,char*);
            nifile = va_arg(argptr,int);
         }
         ier = 0;
         for(;;) 
         {
            oper = va_arg(argptr,int);
            if(oper == 1) 
            {
               iostat = va_arg(argptr,LONG*);
               ier = va_arg(argptr,int);
               fiostatus(iostat,ier);
            }
            else 
            {
               if(lun == INTERNAL) fiointu(ifile,nifile,6);
               else fiolun(lun,6);
               va_end(argptr);
               fioflush();
               return fioerror(1);
            }
         }
      }
   /*****************************************************************************/
   /*                                                                           */
   /* FTNOPEN: FORTRAN Open Statement                                           */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* Executes the FORTRAN OPEN statement when translated via a non-optimized   */
   /* user bias. In addition to the logical unit number, this function takes a  */
   /* variable number of parameters which specify the actual data to be         */
   /* supplied to the open function. The parameter type codes passed to this    */
   /* function are as follows:                                                  */
   /*                                                                           */
   /* Code   Parameter  Description of Use                                      */
   /* ----   ---------  ------------------                                      */
   /*  0     ----       Ends the list of parameters                             */
   /*  1     LONG*      Points to an error return variable                      */
   /*  2     string     Points to the file name                                 */
   /*  3     char*      Points to the status description                        */
   /*  4     char*      Points to access type description                       */
   /*  5     char*      Points to the form description                          */
   /*  6     int        Defines the record length                               */
   /*  7     char*      Points to the blanks treatment description              */
   /*  8     ---        Indicates that the file is readonly                     */
   /*  9     ----       Indicates that the file is to be share                  */
   /* 10     char*      Points to the record type description                   */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if the open went well, else an error code. See the general         */
   /* discussion of FORTRAN I/O capabilities for a listing of the possible      */
   /* error codes.                                                              */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* Initialize the I/O system via the logical unit number, pass the open      */
   /* argument information, finally do the physical open.                       */
   /*                                                                           */
   /*****************************************************************************/

#ifdef VARUNIXV
   int ftnopen(va_alist) va_dcl      /* FORTRAN Open Statement */
#else
      int ftnopen(int lun,...)
#endif
      {
         va_list argptr;
         int oper;
         char* str;
         int len;
         LONG* iostat;
         int ipass;
         int ier;
         char* ifile;              /* Pointer to internal file storage */
         int nifile;               /* Internal file record length */
         char* uname;
         int nuname;
#ifdef VARUNIXV
         int lun;
         va_start(argptr);
         lun = va_arg(argptr,int);
#else
         va_start(argptr,lun);
#endif
         nifile = nuname = 0;
         ifile = uname = NULL;
         if(lun == INTERNAL)            /* This is an internal file */
         {
            ifile = va_arg(argptr,char*);
            nifile = va_arg(argptr,int);
         }
         else if(lun == NAMED)
         {
            lun = fionunit;
            fionunit++;
            uname = va_arg(argptr,char*);
            nuname = va_arg(argptr,int);
         }
         ipass = ier = 0;
         for(;;)
         {
            oper = va_arg(argptr,int);
            if(oper == 1)
            {
               iostat = va_arg(argptr,LONG*);
               ier = va_arg(argptr,int);
               fiostatus(iostat,ier);
            }
            else
            {
               if(!ipass)
               {
                  if(lun == INTERNAL) fiointu(ifile,nifile,0);
                  else fiolun(lun,0);
                  if(uname != NULL)
                  {
                     fiofdata(1,uname,nuname);
                     uname = NULL;
                  }
                  ipass = 1;
               }
               if(!oper)
               {
                  va_end(argptr);
                  fioopen();
                  return fioerror(1);
               }
               str = NULL;
               len = 1;
               if(oper == 6 || oper == 13 || oper == 16) len = va_arg(argptr,int);
               else if(oper != 8 && oper != 9)
               {
                  str = va_arg(argptr,char*);
                  if(oper == 2 || oper == 15 || oper == 17)
                     len = va_arg(argptr,int);
               }
               fiofdata(oper-1,str,len);
            }
         }
      }

   /*****************************************************************************/
   /*                                                                           */
   /* FTNWEF: FORTRAN End File Statement                                        */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* Executes the FORTRAN ENDFILE statement when translated via a non-         */
   /* optimized user bias. In addition to the logical unit number, this         */
   /* function takes a variable number of parameters which specify the actual   */
   /* data to be supplied to the endfile function. The parameter type codes     */
   /* passed to this function are as follows:                                   */
   /*                                                                           */
   /* Code   Parameter  Description of Use                                      */
   /* ----   ---------  ------------------                                      */
   /*  0     ----       Ends the list of parameters                             */
   /*  1     LONG*      Points to an error return variable                      */
   /*                                                                           */
   /* Return value:                                                             */
   /* A zero if the endfile went well, else an error code. See the general      */
   /* discussion of FORTRAN I/O capabilities for a listing of the possible      */
   /* error codes.                                                              */
   /*                                                                           */
   /* See also:                                                                 */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* Initialize the I/O system via the logical unit number, pass the endfile   */
   /* argument information, finally do the physical endfile.                    */
   /*                                                                           */
   /*****************************************************************************/

#ifdef VARUNIXV
   int ftnwef(va_alist) va_dcl       /* FORTRAN End File Statement */
#else
      int ftnwef(int lun,...)
#endif
      {
         va_list argptr;
         int oper;
         LONG* iostat;
         int ier;
         char* ifile;              /* Pointer to internal file storage */
         int nifile;               /* Internal file record length */

#ifdef VARUNIXV
         int lun;
         va_start(argptr);
         lun = va_arg(argptr,int);
#else
         va_start(argptr,lun);
#endif
         nifile = 0;
         ifile = NULL;
         if(lun == INTERNAL)            /* This is an internal file */
         {
            ifile = va_arg(argptr,char*);
            nifile = va_arg(argptr,int);
         }
         ier = 0;
         for(;;)
         {
            oper = va_arg(argptr,int);
            if(oper == 1)
            {
               iostat = va_arg(argptr,LONG*);
               ier = va_arg(argptr,int);
               fiostatus(iostat,ier);
            }
            else
            {
               if(lun == INTERNAL) fiointu(ifile,nifile,6);
               else fiolun(lun,6);
               va_end(argptr);
               fiowef();
               return fioerror(1);
            }
         }
      }
   /*****************************************************************************/
   /*                                                                           */
   /* FTNUNLOCK: FORTRAN Unlock Statement                                       */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* Executes the FORTRAN UNLOCK statement when translated via a non-optimized */
   /* user bias. In addition to the logical unit number, this function takes a  */
   /* variable number of parameters which specify the actual data to be         */
   /* supplied to the unlock operation. The parameter type codes passed to this */
   /* function are as follows:                                                  */
   /*                                                                           */
   /* Code   Parameter  Description of Use                                      */
   /* ----   ---------  ------------------                                      */
   /*  0     ----       Ends the list of parameters                             */
   /*  1     LONG*      Points to an error return variable                      */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if the unlock went well, else an error code. See the general       */
   /* discussion of FORTRAN I/O capabilities for a listing of the possible      */
   /* error codes.                                                              */
   /*                                                                           */
   /* See also:                                                                 */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* Initialize the I/O system via the logical unit number, pass the unlock    */
   /* argument information, finally do the physical unlock.                     */
   /*                                                                           */
   /*****************************************************************************/

#ifdef VARUNIXV
   int ftnunlock(va_alist) va_dcl    /* FORTRAN Unlock Statement */
#else
      int ftnunlock(int lun,...)
#endif
      {
         va_list argptr;
         int oper;
         LONG* iostat;
         int ier;
         char* str;
         int ipass;
         char* ifile;              /* Pointer to internal file storage */
         int nifile;               /* Internal file record length */

#ifdef VARUNIXV
         int lun;
         va_start(argptr);
         lun = va_arg(argptr,int);
#else
         va_start(argptr,lun);
#endif
         nifile = 0;
         ifile = NULL;
         if(lun == NAMED)
         {
            ifile = va_arg(argptr,char*);
            nifile = va_arg(argptr,int);
            fioname(ifile,nifile,0);
            if(fioier == 0) lun = fiocurf->funit;
         }
         if(lun == INTERNAL)            /* This is an internal file */
         {
            ifile = va_arg(argptr,char*);
            nifile = va_arg(argptr,int);
         }
         ier = ipass = 0;
         for(;;)
         {
            oper = va_arg(argptr,int);
            if(oper == 1)
            {
               iostat = va_arg(argptr,LONG*);
               ier = va_arg(argptr,int);
               fiostatus(iostat,ier);
               if(fioier != 0)
               {
                  va_end(argptr);
                  return fioerror(1);
               }
            }
            else
            {
               if(!ipass)
               {
                  if(lun == INTERNAL) fiointu(ifile,nifile,6);
                  else fiolun(lun,6);
                  ipass = 1;
               }
               if(oper == 3)
               {
                  str = va_arg(argptr,char*);
                  fiofdata(2,str,1);
               }
               else
               {
                  va_end(argptr);
                  return fioerror(1);
               }
            }
         }
      }

   /*****************************************************************************/
   /*                                                                           */
   /* FTNLOCK: FORTRAN Lock Statement                                           */
   /*                                                                           */
   /* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
   /*                                                                           */
   /* Author: Fred Goodman                                                      */
   /*                                                                           */
   /* Purpose:                                                                  */
   /*                                                                           */
   /* Executes the FORTRAN LOCK statement when translated via a non-optimized   */
   /* user bias. In addition to the logical unit number, this function takes a  */
   /* variable number of parameters which specify the actual data to be         */
   /* supplied to the lock operation. The parameter type codes passed to this   */
   /* function are as follows:                                                  */
   /*                                                                           */
   /* Code   Parameter  Description of Use                                      */
   /* ----   ---------  ------------------                                      */
   /*  0     ----       Ends the list of parameters                             */
   /*  1     LONG*      Points to an error return variable                      */
   /*                                                                           */
   /* Return value:                                                             */
   /*                                                                           */
   /* A zero if the lock went well, else an error code. See the general         */
   /* discussion of FORTRAN I/O capabilities for a listing of the possible      */
   /* error codes.                                                              */
   /*                                                                           */
   /* See also:                                                                 */
   /*                                                                           */
   /* Process:                                                                  */
   /*                                                                           */
   /* Initialize the I/O system via the logical unit number, pass the lock      */
   /* argument information, finally do the physical lock.                       */
   /*                                                                           */
   /*****************************************************************************/

#ifdef VARUNIXV
   int ftnlock(va_alist) va_dcl      /* FORTRAN Lock Statement */
#else
      int ftnlock(int lun,...)
#endif
      {
         va_list argptr;
         int oper;
         LONG* iostat;
         int ier;
         char* str;
         int ipass;
         char* ifile;              /* Pointer to internal file storage */
         int nifile;               /* Internal file record length */

#ifdef VARUNIXV
         int lun;
         va_start(argptr);
         lun = va_arg(argptr,int);
#else
         va_start(argptr,lun);
#endif
         nifile = 0;
         ifile = NULL;
         if(lun == NAMED)
         {
            ifile = va_arg(argptr,char*);
            nifile = va_arg(argptr,int);
            fioname(ifile,nifile,0);
            if(fioier == 0) lun = fiocurf->funit;
         }
         if(lun == INTERNAL)            /* This is an internal file */
         {
            ifile = va_arg(argptr,char*);
            nifile = va_arg(argptr,int);
         }
         ier = ipass = 0;
         for(;;)
         {
            oper = va_arg(argptr,int);
            if(oper == 1)
            {
               iostat = va_arg(argptr,LONG*);
               ier = va_arg(argptr,int);
               fiostatus(iostat,ier);
               if(fioier != 0)
               {
                  va_end(argptr);
                  return fioerror(1);
               }
            }
            else
            {
               if(!ipass)
               {
                  if(lun == INTERNAL) fiointu(ifile,nifile,6);
                  else fiolun(lun,6);
                  ipass = 1;
               }
               if(oper == 3)
               {
                  str = va_arg(argptr,char*);
                  fiofdata(2,str,1);
               }
               else
               {
                  va_end(argptr);
                  return fioerror(1);
               }
            }
         }
      }

   /*
     ;/hdr/ ************************************************************************
     ;
     ; chrshort: Short Character Representations
     ;
     ; This provider converts short integer values from and to their character
     ; representations. In this context a short integer is a 4-byte integer value.
     ;
     ; Services Available from Provider:
     ;
     ; int  ChrShortValue         Obtain short value from string value
     ; int  ChrShortString        Obtain string value from short value
     ; int  ChrShortFromString    Extract short value from string
     ;
     ;/hdr/ ************************************************************************
   */
#define CHARTYPE                  /* Gives access to character type info */
#define STRGFUNC                  /* Gives access to string services */
   // #include "platform.h"             /* Define the platform hosting this code */
   // #include "chrapi.h"               /* Character manipulation interface */
   /*
     ;/doc/ ***********************************************************************
     ;
     ; ChrShortValue: Obtain Short Value from String Value
     ;
     ; Synopsis of Service:
     ;
     ; #include "chrapi.h"                  Character manipulation interface
     ;
     ; int ChrShortValue                    Obtain short value from string value
     ; (
     ;    char* String                      The string containing the value
     ;    int   nString                     The length of the string
     ;    int   nDecimal                    Number of assumed decimal places
     ; )
     ;
     ; Description of Service:
     ;
     ; This service converts an alphanumeric string to a short integer 4-byte value.
     ; If the string contains no decimal point the the value is increased by the
     ; power of ten indicated. If the string contains decimal places then they must
     ; match the number specified. Note that this service accepts negative value
     ; representations; however, a leading minus sign must be used.
     ;
     ; Properties of Service:
     ;
     ; String     Points to the string containing the integer value to be
     ;            determined. This string is assumed to be nul-terminated
     ;            only if the nString property is zero.
     ;
     ; nString    The length of the string represention. If this is zero, then the
     ;            String property is assumed to encompass the value and to be
     ;            nul-termined.
     ;
     ; nDecimal   The number of assumed decimal places in the value. The string
     ;            must contain either no decimal places of exactly this many 
     ;            decimal places.
     ;
     ; Return Value from Service:
     ;
     ; The service returns the computed short value. If the representation was not
     ; well-formed, then the ChrError global returns one of the following nonzero
     ; error codes:
     ;
     ; Code              Meaning
     ; -------------     ---------
     ; CHRERR_WRONGDEC   The string had the wrong number of decimal places
     ; CHRERR_BADDIGITS  The string contained non-numeric digits
     ;
     ;/doc/ ************************************************************************
   */
#ifdef FPROTOTYPE
   int ChrShortValue(char* String,int nString,int nDecimal)
#else
      int ChrShortValue(String,nString,nDecimal)
      char* String;                  /* The string containing the value */
   int   nString;                 /* The length of the string */
   int   nDecimal;                /* Number of assumed decimal places */
#endif
   {
      char* str;                /* Local input string pointer */
      int minus;                /* Sign indicator */
      int ivalue;               /* Integer value of the string */
      int iString;              /* Dummy index */
      int iDecimal;             /* Keeps track of decimal point and digits */

      /*-------------------------------------------------------------------------
        ;
        ; Step 1: Initialize the local control variables needed for the conversion.
        ;
        ;------------------------------------------------------------------------*/

      if(nString == 0) nString = strlen(String);
      ChrError = 0;
      str = String;
      minus = 0;
      iDecimal = -1;

      /*-------------------------------------------------------------------------
        ;
        ; Step 2: Examime the initial character of the string to determine if there
        ; is a leasing sign. If it is a minus sign record the fact that a negative
        ; value is to be returned.
        ;
        ;------------------------------------------------------------------------*/

      if(*str == '-' || *str == '+')
      {
         if(*str == '-') minus = 1;
         str++;
         nString--;
      }

      /*--------------------------------------------------------------------------
        ;
        ; Step 3: Compute the actual value by summing the digits with their
        ; appropriate powers of ten. Also check for and record any decimal places.
        ;
        ;-------------------------------------------------------------------------*/

      ivalue = 0;
      for(iString = 0; iString < nString; iString++)
      {
         if(*str == '.' && iDecimal < 0) iDecimal = 0; 
         else
         {
            if(!(ChrTypeFlags[(int)*str] & CNUMBER)) break;
            ivalue = ivalue * 10 + ChrInformation[(int)*str];
            if(iDecimal >= 0) iDecimal++;
         }
         str++;
      }
      if(minus) ivalue = -ivalue;

      /*------------------------------------------------------------------------
        ;
        ; Step 4: Make certain that there are no bad characters in the input
        ; representation. If there are set the error code.
        ;
        ;-----------------------------------------------------------------------*/

      while(iString < nString)
      {
         if(!(ChrTypeFlags[(int)*str] & CWSPACE))
         {
            ChrError = CHRERR_BADDIGITS;
            break;
         }
         str++;
         iString++;
      }

      /*-------------------------------------------------------------------------
        ;
        ; Step 5: If the input representation was well-formed make certain that
        ; if decimal digits were included, that there were the right number.
        ;
        ;-------------------------------------------------------------------------*/

      if(ChrError == 0)
      {
         if(iDecimal >= 0 && iDecimal != nDecimal)
         {
            ChrError = CHRERR_WRONGDEC;
         }
         else if(iDecimal < 0 && nDecimal > 0) ivalue *= ChrTen[nDecimal-1];
      }
      return ivalue;
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; ChrShortString: Obtain String Value from Short Value
     ;
     ; Synopsis of Service:
     ;
     ; #include "chrapi.h"                  Character manipulation interface
     ;
     ; int ChrShortString                   Obtain short value from string value
     ; (
     ;    int   Value                       The value to be converted
     ;    char* String                      Returns the string form of the value
     ;    int   nDecimal                    Number of assumed decimal places
     ; )
     ;
     ; Description of Service:
     ;
     ; This service converts an a short integer 4-byte value into a character
     ; string.
     ;
     ; Properties of Service:
     ;
     ; Value      The value to be converted
     ;
     ; String     Returns the character representation of the value in nul-
     ;            terminated string form.
     ;
     ; nDecimal   The number of assumed decimal places in the value.
     ;
     ; Return Value from Service:
     ;
     ; The length of the character representation, not counting the nul.
     ;
     ;/doc/ ************************************************************************
   */
#ifdef FPROTOTYPE
   int ChrShortString(int Value,char* String,int nDecimal)
#else
      int ChrShortString(Value,String,nDecimal)
      int   Value;                   /* The value to be converted */
   char* String;                  /* Returns the string form of the value */
   int   nDecimal;                /* Number of assumed decimal places */
#endif
   {
      int minus;                /* If nonzero then the value was negative */
      int iv;                   /* Intermediate value of a digit */
      int ndigit;               /* Numbers of digits in result string */
      int i;                    /* Dummy counter used for string reversal */
      int j;                    /* Dummy counter used for string reversal */

      /*-------------------------------------------------------------------------
        ;
        ; Step 1: If the value is negative, record that fact and convert if to
        ; positive form.
        ;
        ;-------------------------------------------------------------------------*/

      if(Value < 0)
      {
         minus = 1;
         Value = -Value;
      }
      else minus = 0;

      /*-------------------------------------------------------------------------
        ;
        ; Step 2: Form the digits making up the representation in reverse order.
        ;
        ;-------------------------------------------------------------------------*/

      ndigit = 0;
      do
      {
         iv = ChrDigits[(Value % 10)];
         *(String+ndigit++) = (char)(iv);
      }
      while((Value /= 10) > 0);
      if(ndigit == 1 && *String == 'U') *String = '0';

      /*-------------------------------------------------------------------------
        ;
        ; Step 3: Reverse the order of the digits so they are in the expected form.
        ;
        ;------------------------------------------------------------------------*/

      for(i = 0, j = ndigit-1; i < j; i++, j--)
      {
         iv = *(String+i);
         *(String+i) = *(String+j);
         *(String+j) = (char)(iv);
      }
      *(String+ndigit) = '\0';

      /*-------------------------------------------------------------------------
        ;
        ; Step 4: Make any adjustments needed to show any requested decimal places.
        ;
        ;------------------------------------------------------------------------*/

      if(nDecimal)
      {
         if(ndigit <= nDecimal)
         {
            iv = nDecimal-ndigit+1;
            ChrShiftRight(String,iv,'0');
            ndigit += iv;
         }
         ChrShiftRight(String+ndigit-nDecimal,1,'.');
         ndigit++;
      }

      /*--------------------------------------------------------------------------
        ;
        ; Step 5: Add a leading minus sign if the value was originally negative.
        ;
        ;-------------------------------------------------------------------------*/

      if(minus)
      {
         ChrShiftRight(String,1,'-');
         ndigit++;
      }
      return ndigit;
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; ChrShortFromString: Extract Short Value from string
     ;
     ; Synopsis of Service:
     ;
     ; #include "chrapi.h"                  Character processing interface
     ;
     ; int ChrShortFromString               Extract short value from string
     ; (
     ;    char** s,                         Points to location of string pointer
     ;    int    blank                      Contains the blank treatment convention
     ; )
     ;
     ; Description of Service:
     ;
     ; This service converts an alphanumeric string to an integer value via a
     ; pointer to a pointer to a string. That pointer is then updated to point
     ; to the position beyond the end of the integer representation.
     ;
     ; Properties of Service:
     ;
     ; s        Points to the location of a pointer to the start of the string.
     ;          This location is updated to point immediately beyond the last
     ;          character of the integer value.
     ;
     ; blank    Contains the blanks convention to be used. If it has a value of
     ;          zero, then blanks terminate the string like any other nonnumeric
     ;          character; if positive, then blanks are simply ignored; and if
     ;          negative, then blanks have the value of "zero".
     ;
     ; Return Value from Service:
     ;
     ; The service returns the computed value.
     ;
     ;/doc/ ************************************************************************
   */
// #ifndef WIN32
#ifdef FPROTOTYPE
   int ChrShortFromString(const char** s,int blank)
#else
      int ChrShortFromString(s,blank)
      const char** s;                          /* Pointer to the string pointer */
   int blank;                         /* Blanks convention */
#endif
   {
      const char* str;                /* Local input string pointer */
      int minus;                /* Sign indicator */
      int iv;                   /* Integer value of the string */

      /*--------------------------------------------------------------------------
        ;
        ; Step 1: Obtain the actual input string address for local use.
        ;
        ;-------------------------------------------------------------------------*/

      str = *s;

      /*--------------------------------------------------------------------------
        ;
        ; Step 2: Examine the first character in the input string to determine
        ; if it is a sign character and if so determine its type.
        ;
        ;-------------------------------------------------------------------------*/

      if(*str == '-')
      {
         minus = 1;
         str++;
      }
      else minus = 0;
      if(*str == '+') str++;

      /*--------------------------------------------------------------------------
        ;
        ; Step 3: Move through the string until a nonnumeric character is
        ; encountered and compute the value of the integer string. Note the
        ; effect of the blanks convention variable on this computation.
        ;
        ;-------------------------------------------------------------------------*/

      iv = 0;
      for(;;)
      {
         if(*str == ' ' && blank != 0)
         {
            if(blank < 0) iv = iv * ChrModulo;
         }
         else if(ChrTypeFlags[(int)*str] & CNUMBER) 
         {
            iv = iv * ChrModulo + ChrInformation[(int)*str];
         }
         else break;
         str++;
      }
      if(minus) iv = -iv;

      /*--------------------------------------------------------------------------
        ;
        ; Step 4: Reset the pointer to the pointer to the start of the string so
        ; that it points to the first character beyond the end of the string,
        ; and return the integer value.
        ;
        ;-------------------------------------------------------------------------*/

      *s = str;
      return iv;
   }

// #endif

#ifdef FPROTOTYPE
   int ChrShortFromString(char** s,int blank)
#else
      int ChrShortFromString(s,blank)
      char** s;                          /* Pointer to the string pointer */
   int blank;                         /* Blanks convention */
#endif
   {
      char* str;                /* Local input string pointer */
      int minus;                /* Sign indicator */
      int iv;                   /* Integer value of the string */

      /*--------------------------------------------------------------------------
        ;
        ; Step 1: Obtain the actual input string address for local use.
        ;
        ;-------------------------------------------------------------------------*/

      str = *s;

      /*--------------------------------------------------------------------------
        ;
        ; Step 2: Examine the first character in the input string to determine
        ; if it is a sign character and if so determine its type.
        ;
        ;-------------------------------------------------------------------------*/

      if(*str == '-')
      {
         minus = 1;
         str++;
      }
      else minus = 0;
      if(*str == '+') str++;

      /*--------------------------------------------------------------------------
        ;
        ; Step 3: Move through the string until a nonnumeric character is
        ; encountered and compute the value of the integer string. Note the
        ; effect of the blanks convention variable on this computation.
        ;
        ;-------------------------------------------------------------------------*/

      iv = 0;
      for(;;)
      {
         if(*str == ' ' && blank != 0)
         {
            if(blank < 0) iv = iv * ChrModulo;
         }
         else if(ChrTypeFlags[(int)*str] & CNUMBER) 
         {
            iv = iv * ChrModulo + ChrInformation[(int)*str];
         }
         else break;
         str++;
      }
      if(minus) iv = -iv;

      /*--------------------------------------------------------------------------
        ;
        ; Step 4: Reset the pointer to the pointer to the start of the string so
        ; that it points to the first character beyond the end of the string,
        ; and return the integer value.
        ;
        ;-------------------------------------------------------------------------*/

      *s = str;
      return iv;
   }


   /*
     ;/hdr/ ************************************************************************
     ;
     ; Copyright 1986-1998 Promula Development Corporation ALL RIGHTS RESERVED
     ;
     ; name of module: FioForm.c -- Fortran I/O, Format Processing
     ;
     ; global symbols defined:
     ;
     ; useful notes and assumptions:
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;         author: (LCC) Lois Carver, Promula Development Corporation
     ;
     ; edit 08/24/98 LCC: Converted headers and comments to new standards.
     ;
     ; edit 06/17/98 FKG: Changed names of global symbols to conform to the 
     ;                    Promula version 7.08 API naming conventions. 
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00
     ;
     ;/hdr/ ************************************************************************
   */
#define CHARTYPE                  /* Gives access to character type info */
#define PRIOFUNC                  /* Gives access to PROMULA I/O functions */
#define VARARGUS                  /* Gives access to variable arguments */
   // #include "platform.h"             /* Define platform hosting this code */
   // #include "pfclib.h"               /* Define PROMULA function library symbols */
   // #include "chrapi.h"               /* Define character manipulation interface */

#ifdef FPROTOTYPE
   int  fioerror(int clear);
#else
   extern int  fioerror();           /* Do requested error processing */
#endif

   char   fiobfmt[40];               /* Business formatting string */
   char*  fiocfmt = NULL;            /* Current format position */
   char** fiofrmt = NULL;            /* Current format address list */
   int    fiofspec = 0;              /* Current format specification */
   int    fiofstat = 0;              /* Current processing state */
   int    fioifmt = 0;               /* Number of current format entry */
   int    fioiresc = 0;              /* Number of current rescan entry */
   int    fiomaxc = 0;               /* Maximum characters in output line */
   int    fionfmt = 0;               /* Number of entries in format list */
   int    fionpren = 0;              /* Parenthetical nesting */
   int    fionunit = 30000;          /* Starting unit number for named files */
   char*  fiosadr = NULL;            /* Format string location */
   char*  fiorscan = NULL;           /* Current rescan positition */
   int    fiorscnt = 0;              /* Rescan count value */
#ifdef FPROTOTYPE
   int (*fiovfunc)(int);             /* User's variable format function */
#else
   int (*fiovfunc)();                /* User's variable format function */
#endif
   int    fiovfnum = 0;              /* Number of branches in user's function */

   extern FIOFILE* fiocurf;          /* Pointer to current FORTRAN file */
   extern int      fioconv;          /* Runtime convention flags */
   extern int      fioefc;           /* E-format exponent character */
   extern int      fioefw;           /* E-format exponent width */
   extern int      fioier;           /* Code for actual error encountered */
   extern int      fioiwd;           /* Display width */
   extern int      fiondec;          /* Display decimal specification */
   extern int      fioscal;          /* Floating point scale factor */

#ifdef FPROTOTYPE
   int fiofmtv(void);
#else
   extern int fiofmtv();
#endif

   static int vfmtval[20];

   /*
     ;/doc/ ***********************************************************************
     ;
     ; name of routine:  FIOVFORM -- Store FORTRAN Variable FORMAT Values
     ;
     ; Saves a pointer to the user's variable format function and the number of its
     ; branches.
     ;
     ; calling parameters:
     ;
     ; return parameters:
     ;
     ; useful notes and assumptions:
     ;
     ; Simply save the argument values in the global variables.
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00
     ;
     ;/doc/ ************************************************************************
   */

#ifdef FPROTOTYPE
   void fiovform(int (*vf)(int),int nbranch)
#else
      void fiovform(vf,nbranch)
      int (*vf)();
   int nbranch;
#endif
   {
      fiovfunc = vf;
      fiovfnum = nbranch;
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; name of routine:  FIOFWSP -- Skip Format White Space
     ;
     ; This function is a utility used by the FORMAT processing functions to skip
     ; over white space within the specification string. White space consists of
     ; blanks and boundaries between the independent lines of the specification.
     ;
     ; calling parameters:
     ;
     ; return parameters:  None, the function is void. The effect of its processing
     ;                     is reflected in the various global format control
     ;                     variables.
     ;
     ; useful notes and assumptions:
     ;
     ; Element       Description of use
     ; -------       ------------------
     ; fionfmt       Move to next token within the format specification
     ; fiofmtv       Move to next digit in a specification constant
     ; fiofini       Move to opening parenthesis of specification
     ;
     ; Simply increment the format control variables until a nonblank character
     ; is encountered.
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00
     ;
     ;/doc/ ************************************************************************
   */

#ifdef FPROTOTYPE
   void fiofwsp(void)
#else
      void fiofwsp()
#endif
      {
      fiofwsp1:
         do 
         {
            fiocfmt++;
         } 
         while(*fiocfmt == ' ');
         if(!*fiocfmt)
         {
            if(++fioifmt < fionfmt)
            {
               fiocfmt = fiofrmt[fioifmt];
               if(*fiocfmt == ' ') goto fiofwsp1;
            }
         }
      }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; name of routine:  FIOFINI -- Initialize a FORTRAN Format
     ;
     ; This function initializes the FORMAT environment needed by the FORTRAN style
     ; input/output statements. There is a minor problem associated with the
     ; processing of FORTRAN style FORMATS in C, having to do with "maximum string
     ; length". In theory, since a C string consists of a sequence of nonzero
     ; characters terminated by a zero byte, it can be of any length desired.
     ; Unfortunately, all C compilers place a limit on the maximum length that a
     ; string constant may have, typically 256. Now it is not at all unusual for
     ; FORMAT specification strings to be very long, and they are typically defined
     ; as constants at compile time. Fortunately, though length of an individual
     ; string constant is limited, C has a very neat notation for defining a
     ; constant set of pointers to constant strings. Thus, a long format string can
     ; be writen very conveniently as a sequence of individual lines as follows:
     ;
     ;     static char fmt01[ ] = {
     ;         "(T2,20A4,//,T2,'MASS (M) UNITS = ',2A4,/,"
     ;         "T2,'LENGTH (L) UNITS = ',2A4,/,"
     ;         "T2,'TIME (T) UNITS = ',2A4,//)"
     ;     };
     ;
     ; Using this notation, indefinitely long FORMAT specification strings can be
     ; written as a sequence of blocked lines. Note that the FORMAT routines ignore
     ; the breaks between the lines, so they may be broken in way. Simple FORMAT
     ; specification strings may of course still be written as standard string
     ; constants such as the following:
     ;
     ;     static char fmt02[ ] = "(F10.0)";
     ;
     ; The FORMAT specification string itself has the same content as a FORMAT would
     ; have in a standard FORTRAN environment. It must begin with a left-parenthesis
     ; and end with a right-parenthesis. See the general discussion of FORTRAN I/O
     ; capabilities for a detailed description of the format specifications. There
     ; are two parameters to be supplied for this function as follows:
     ;
     ; Name    Description of use
     ; ---     ------------------
     ; fmt     Points to the format control  string.  As discussed above it may
     ;         either be a  pointer to a series of strings or it may be a simple
     ;         string  pointer.  It may also  be a NULL, meaning either that free-
     ;         format is to be used or  meaning that a "binary" operation is to be
     ;         performed.
     ;
     ; nfmt    Provides additional information about the above. If "fmt" is NULL
     ;         and "nfmt" is zero then an unformatted binary type operation is
     ;         being performed. If "fmt" is NULL and "nfmt" is nonzero, then a
     ;         free-form "formatted" operation is being performed. If "fmt" is
     ;         not NULL and "nfmt" is zero, then a single string format is being
     ;         processed; else the specification consists of a sequence of "nfmt"
     ;         lines.
     ;
     ; Once this function is called, the format specification string remains in
     ; effect until either this function is called again, or until function
     ; "fiofend" is called.
     ;
     ; calling parameters:
     ;
     ; return parameters:
     ;
     ; A zero if the format appears well-formed, else an error code. See the general
     ; discussion of FORTRAN I/O capabilities for a listing of the possible error
     ; codes.
     ;
     ; useful notes and assumptions:
     ;
     ; Element       Description of use
     ; -------       ------------------
     ; FORTRAN       WRITE statement
     ;               READ statement
     ;
     ; Simply copy the parameters into the global current format control variables
     ; and initialize the remaining format controls. If the specification string
     ; does not begin with a left-parenthesis, establish error code 201.
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00
     ;
     ;/doc/ ************************************************************************
   */

#ifdef FPROTOTYPE
   int fiofini(char** fmt,int nfmt)
#else
      int fiofini(fmt,nfmt)
      char** fmt;                    /* Pointer to format or format pointers */
   int nfmt;                      /* Num of format pointers or unformat type */
#endif
   {
      fiofrmt = fmt;
      if(fmt == NULL)
      {
         fionfmt = nfmt;
         fiocfmt = NULL;
         goto fiofini9;
      }
      if(nfmt == 0)
      {
         fiocfmt = (char*)fmt;
         fionfmt = 1;
      }
      else
      {
         fiocfmt = fiofrmt[0];
         fionfmt = nfmt;
      }
      fioifmt = fiofstat = fionpren = fioscal = 0;
      fiomaxc = -1;
      fiobfmt[0] = '\0';
      if(*fiocfmt == ' ') fiofwsp();
      if(*fiocfmt == '(')
      {
         fiofwsp();
         fiorscan = fiocfmt;
         fioiresc = fioifmt;
         fiorscnt = -1;
         goto fiofini9;
      }
   fiofini9:
      return fioerror(0);
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; name of routine:  FIOFEND -- End Format Processing
     ;
     ; Clears the format control variables to end processing with the current format.
     ;
     ; calling parameters:
     ;
     ; return parameters:  None, the function is void.
     ;
     ; useful notes and assumptions:
     ;
     ; Element       Description of use
     ; -------       ------------------
     ; fiorln        Ends input format processing.
     ; fiowln        Ends output format processes
     ;
     ; Simple set the number of format lines to zero and if the operation was to
     ; an internal string variable mark the current file structure as unused.
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00
     ;
     ;/doc/ ************************************************************************
   */

#ifdef FPROTOTYPE
   void fiofend(void)
#else
      void fiofend()
#endif
      {
         fionfmt = 0;
         if((fiocurf->fstat & FINTERNAL) && (fiocurf->funit != INTERNAL) &&
            (fiocurf->fassob == NULL)) fiocurf->fstat |= FUNUSED;
      }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; name of routine:  FIOVFVAL -- Get Variable FORMAT Value
     ;
     ; Determines which variable format value is desired and then returns its
     ; current setting.
     ;
     ; calling parameters:
     ;
     ; return parameters:  The value of the specified variable format value.
     ;
     ; useful notes and assumptions:
     ;
     ; Element       Description of use
     ; -------       ------------------
     ; fiofmtv       Determine variable format value
     ;
     ; Simply obtain the sequence number of the value from the current position in
     ; the FORMAT specification. The look up the current setting for that value.
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00
     ;
     ;/doc/ ************************************************************************
   */

#ifdef FPROTOTYPE
   int fiovfval(void)
#else
      int fiovfval()
#endif
      {
         int inum;                 /* The sequence number of the value */

         fiofwsp();
         inum = fiofmtv();
         fiofwsp();
         if(inum < 0 || inum >= fiovfnum) inum = 0;
         else inum = (*fiovfunc)(inum);
         return inum;
      }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; name of routine:  FIOFMTV -- Compute FORMAT Value
     ;
     ; Computes the value of an integer constant in the FORMAT statement, and
     ; updates the current format position so that it points to the first nonblank
     ; character beyond the end of the value. Note that if the current character in
     ; the format is nonnumeric when this function is called, then this function
     ; does not move the current position and returns a zero value.
     ;
     ; calling parameters:
     ;
     ; return parameters:  The value of the integer constant or a zero if there was
     ;                     no constant.
     ;
     ; useful notes and assumptions:
     ;
     ; Element       Description of use
     ; -------       ------------------
     ; fionxtf       Determines the counts, widths, and decimal places associated
     ;               with the format specifications.
     ;
     ; Using successive multiplications by 10 and additions evaluate the constant
     ; until a nonnumeric character is encountered. Note that since "fiofwsp" is
     ; used to move from character to character in the format, the constant may span
     ; separate format lines.
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00
     ;
     ;/doc/ ************************************************************************
   */

#ifdef FPROTOTYPE
   int fiofmtv(void)
#else
      int fiofmtv()
#endif
      {
         int inum;                 /* The value of the constant */
         int minus;                /* Leading sign indicator */

         if(*fiocfmt == '<') return fiovfval();
         inum = minus = 0;
         if(*fiocfmt == '-')
         {
            minus = 1;
            fiofwsp();
         }
         else if(*fiocfmt == '+') fiofwsp();
         for(;;)
         {
            if(ChrTypeFlags[(int)*fiocfmt] & CNUMBER)
               inum = inum*10+ChrInformation[(int)*fiocfmt];
            else
            {
               if(minus) inum = -inum;
               return inum;
            }
            fiofwsp();
         }
      }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; name of routine:  FIONXTF -- Get Next Format Specification
     ;
     ; Gets the next format specification from the format list and sets the external
     ; variables (fiofspec, fioiwd, and fiondec) to indicate what it is. The above
     ; variables are set as follows:
     ;
     ;
     ;      Specification  Type    fioiwd    fiondec
     ;      -------------  ----    ------    -------
     ;      free form         0         0          0
     ;      nH                1         n          0
     ;      "c1..cn"          1         n          "
     ;      'c1..cn'          1         n          '
     ;      *c1..cn*          1         n          *
     ;      nX                2         n         --
     ;      TRn               2         n         --
     ;      Tn                3         n         --
     ;      TLn               4         n         --
     ;      SS                5        --         --
     ;      SP                6        --         --
     ;      BN                7        --         --
     ;      BZ                8        --         --
     ;      /                 9        --         --
     ;      nP               10         n         --
     ;      )                11        ---        --
     ;      :                -1        ---        --
     ;      $                -2        ---        --
     ;      Aw               12         w         --
     ;      Lw               13         w         --
     ;      Iw               14         w         --
     ;      Fw.d             15         w          d
     ;      Dw.d             16         w          d
     ;      Ew.d             17         w          d
     ;      Gw.d             18         w          d
     ;      B'ssss'          19        --         --
     ;      Q                20        --         --
     ;
     ; calling parameters:
     ;
     ; return parameters:  None, the function is void.
     ;
     ; useful notes and assumptions:
     ;
     ; Element       Description of use
     ; -------       ------------------
     ; fiofout       Obtain next format specification for output processing.
     ; fiofinp       Obtain next format specification for input processing. 
     ;
     ; See detailed steps below.
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00
     ;
     ;/doc/ ************************************************************************
   */

#ifdef FPROTOTYPE
   void fionxtf(void)
#else
      void fionxtf()
#endif
      {
         char c;                   /* Temporary character value */
         static int delim = 0;          /* String delimeter or count */
         char* fadr;               /* Temporary address in format */
         static char fmtchr[ ] =        /* Format control characters */
            ")\"'*HXTSB/P(ALIFDEG:$OZQ";
         int frec;                 /* Temporary format record number */
         static int gcount[5];          /* Parenthetical group count */
         static int gentry[5];          /* Parenthetical entry number */
         static char* gform[5];         /* Parenthetical group position */
         static int nrepeat = 0;        /* Current repeat count */
         int ic;                   /* Temporary counter */
         int iv;                   /* Temporary value */
         int nv;                   /* Initial field count value */

         /*--------------------------------------------------------------------------
           ;
           ; Step 1: If there is no active format specification simply set the type to
           ; zero and end local processing; else branch according to the current format
           ; processing state:
           ;
           ; Step  Description of processing state
           ; ----  -------------------------------
           ;   2   Ready to find the next specification
           ;   3   Within a delimeted string which spans format entries
           ;   4   Within a holerith string with spans format entries
           ;   5   Within a repeatable format specification
           ;
           ;-------------------------------------------------------------------------*/

         if(!fionfmt)
         {
            fiofspec = fiondec = fioiwd = 0;
            goto fionxtf9;
         }
         switch(fiofstat)
         {
         case 0: goto fionxtf2;
         case 1: goto fionxtf3;
         case 2: goto fionxtf4;
         case 3: goto fionxtf5;
         }

         /*--------------------------------------------------------------------------
           ;
           ; Step 2: The next specification is now needed. Move forward in the
           ; specification skipping over all intervening blanks and commas. Then
           ; obtain the specification type and process it accordingly.
           ;
           ;-------------------------------------------------------------------------*/

      fionxtf2:
         while(*fiocfmt == ',') fiofwsp();
         c = *fiocfmt;
         if(c == '-' || c == '+' || c == '<' || (ChrTypeFlags[(int)c] & CNUMBER))
         {
            nv = iv = fiofmtv();
            if(iv == 0) iv = 1;
            c = *fiocfmt;
         }
         else
         {
            nv = 0;
            iv = 1;
         }
         if(ChrTypeFlags[(int)c] & CLOWERC) c = ChrAlphabet[ChrInformation[(int)c]-1];
         for(ic = 0; fmtchr[ic]; ic++) if(c == fmtchr[ic]) break;
         if(!fmtchr[ic])
         {
            fioier = ENXF_EFS;
            goto fionxtf9;
         }
         switch(ic)
         {
         case 0:                        /* right parenthesis */
            if(!fionpren)
            {
               fiocfmt = fiorscan;
               fioifmt = fioiresc;
               if(fiorscnt >= 0)
               {
                  gcount[fionpren] = fiorscnt;
                  gform[fionpren] = fiocfmt;
                  gentry[fionpren] = fioifmt;
                  fionpren++;
               }
               fiofspec = 11;
               goto fionxtf9;
            }
            else
            {
               if(gcount[fionpren-1] > 0)
               {
                  gcount[fionpren-1]--;
                  fiocfmt = gform[fionpren-1];
                  fioifmt = gentry[fionpren-1];
               }
               else
               {
                  fionpren--;
                  fiofwsp();
               }
            }
            goto fionxtf2;

         case 1: case 2: case 3:        /* delimited character string */
            fiocfmt++;
            delim = c;
            goto fionxtf3;

         case 4:                        /* a holerith string */
            fiocfmt++;
            delim = iv;
            goto fionxtf4;

         case 5:                        /* X specification */
            fiofwsp();
            fiofspec = 2;
            fioiwd = iv;
            goto fionxtf9;

         case 6:                        /* T specification */
            fiofwsp();
            c = *fiocfmt;
            if(c == 'R' || c == 'r')
            {
               fiofspec = 2;
               fiofwsp();
            }
            else if(c == 'L' || c == 'l')
            {
               fiofspec = 4;
               fiofwsp();
            }
            else fiofspec = 3;
            fioiwd = fiofmtv();
            if(!fioiwd) fioier = ENXF_BTF;
            goto fionxtf9;
            
         case 7:                        /* S specification */
            fiofspec = 5;
            fiondec = 0;
            fiofwsp();
            c = *fiocfmt;
            if(c == 'S' || c == 's')
            {
               fiondec = 1;
               fiofwsp();
            }
            else if(c == 'P' || c == 'p')
            {
               fiofspec = 6;
               fiofwsp();
            }
            goto fionxtf9;

         case 8:                        /* B specification of business format */
            fiofwsp();
            c = *fiocfmt;
            if(c == 'N' || c == 'n') fiofspec = 7;
            else if(c == 'Z' || c == 'z') fiofspec = 8;
            else if(c == '\'')
            {
               for(ic = 0; ic < 40; ic++)
               {
                  fiofwsp();
                  if(*fiocfmt == '\'') break;
                  fiobfmt[ic] = *fiocfmt;
               }
               if(ic == 40) fioier = ENXF_BUS;
               else
               {
                  fiobfmt[ic] = '\0';
                  fiofspec = 19;
               }
            }
            else fioier = ENXF_BBF;
            fiofwsp();
            goto fionxtf9;

         case 9:                        /* / specification */
            fiofspec = 9;
            fiofwsp();
            goto fionxtf9;

         case 10:                       /* P specification */
            fioiwd = nv;
            fiofspec = 10;
            fiofwsp();
            goto fionxtf9;

         case 11:                       /* ( A subgroup */
            fiofwsp();
            if(fionpren == 0)
            {
               fiorscan = fiocfmt;
               fioiresc = fioifmt;
               fiorscnt = iv - 1;
            }
            gcount[fionpren] = iv - 1;
            gform[fionpren] = fiocfmt;
            gentry[fionpren] = fioifmt;
            fionpren++;
            goto fionxtf2;

         case 19:                       /* A colon */
            fiofspec = -1;
            fiofwsp();
            goto fionxtf9;

         case 20:                       /* A dollar sign */
            fiofspec = -2;
            fiofwsp();
            goto fionxtf9;

         case 23:
            fiofspec = 20;
            fiofwsp();
            goto fionxtf9;

         default:                       /* Value editing specification */
            fiofspec = ic;
            nrepeat = iv;
            fiofwsp();
            fioiwd = fiofmtv();
            fiondec = 0;
            if(fiofspec > 13)
            {
               if(*fiocfmt == '.')
               {
                  fiofwsp();
                  c = *fiocfmt;
                  if((ChrTypeFlags[(int)c] & CNUMBER) || (c == '<')) fiondec = fiofmtv(); 
               }
               else if(fiofspec == 14) fiondec = -1;
               if(fiofspec > 20)
               {
                  fioefw = fiondec;
                  fiondec = 20 - fiofspec;
                  fiofspec = 12;
               }
               if(fiofspec > 15)
               {
                  frec = fioifmt;
                  fadr = fiocfmt;
                  c = *fiocfmt;
                  if(c == 'E' || c == 'e') fioefc = 'E'; 
                  else if(c == 'D' || c == 'd') fioefc = 'D';
                  else goto fionxt2a;
                  fiofwsp();
                  c = *fiocfmt;
                  if((ChrTypeFlags[(int)c] & CNUMBER) || (c == '<'))
                  {
                     fioefw = fiofmtv(); 
                     if(*fiocfmt != '.') goto fionxtf5;
                  }
               fionxt2a:
                  fioefw = 2;
                  if(fiofspec == 16) fioefc = 'D';
                  else fioefc = 'E';
                  fiocfmt = fadr;
                  fioifmt = frec;
               }
            }
            goto fionxtf5;
         }

         /*--------------------------------------------------------------------------
           ;
           ; Step 3: We are within a delimeted string with may span multiple format
           ; entries. Obtain the currrent portion of the string.
           ;
           ;-------------------------------------------------------------------------*/

      fionxtf3:
         if(!*fiocfmt)
         {
            if(++fioifmt == fionfmt)
            {
               fioier = ENXF_DEL;
               goto fionxtf9;
            }
            fiocfmt = fiofrmt[fioifmt];
         }
         fiosadr = fiocfmt;
         fioiwd = 0;
         while(*fiocfmt && (*fiocfmt != delim || *(fiocfmt+1) == delim))
         {
            if(*fiocfmt == delim && *(fiocfmt+1) == delim)
            {
               fioiwd++;
               fiocfmt++;
            }
            fioiwd++;
            fiocfmt++;
         }
         if(!*fiocfmt) fiofstat = 1;
         else
         {
            fiofwsp();
            fiofstat = 0;
         }
         fiofspec = 1;
         fiondec = delim;
         goto fionxtf9;

         /*--------------------------------------------------------------------------
           ;
           ; Step 4: We are within a holerith string which may span multiple entries.
           ; Obtain the current portion of the string.
           ;
           ;-------------------------------------------------------------------------*/

      fionxtf4:
         if(!*fiocfmt)
         {
            if(++fioifmt == fionfmt)
            {
               fioier = ENXF_HOL;
               goto fionxtf9;
            }
            fiocfmt = fiofrmt[fioifmt];
         }
         fiosadr = fiocfmt;
         fioiwd = 0;
         while(*fiocfmt && fioiwd < delim)
         {
            fioiwd++;
            fiocfmt++;
         }
         if(fioiwd < delim)
         {
            fiofstat = 2;
            delim -= fioiwd;
         }
         else
         {
            fiocfmt--;
            fiofwsp();
            fiofstat = 0;
         }
         fiofspec = 1;
         fiondec = 0;
         goto fionxtf9;

         /*--------------------------------------------------------------------------
           ;
           ; Step 5: We are within a repeating group.
           ;
           ;-------------------------------------------------------------------------*/

      fionxtf5:
         if((--nrepeat) != 0) fiofstat = 3;
         else fiofstat = 0;

         /*--------------------------------------------------------------------------
           ;
           ; Step 6: The next specification has been obtained. Check for an error code
           ; and if set do the requested error processing. The end local processing.
           ;
           ;-------------------------------------------------------------------------*/

      fionxtf9:
         fioerror(0);
      }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; name of routine:  FIOVFMT -- FORTRAN I/O Variable Format Values
     ;
     ; Store a set of variable format values in a local vector so that they can
     ; be passed to the actual FORMAT processing functions via the function fiovfmt.
     ;
     ; calling parameters:
     ;
     ; return parameters:  None.
     ;
     ; useful notes and assumptions:
     ;
     ; Simply store the values and set the global function pointer to the local
     ; function which returns those values.
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00
     ;
     ;/doc/ ************************************************************************
   */

#ifdef FPROTOTYPE
   static int getval(int iv)
#else
      static int getval(iv)
      int iv;
#endif
   {
      return vfmtval[iv];
   }

#ifdef VARUNIXV
   void fiovfmt(va_alist)va_dcl
#else
      void fiovfmt(int nval,...)
#endif
      {
         va_list argptr;
         int i;
#ifdef VARUNIXV
         int nval;
         va_start(argptr);
         nval = va_arg(argptr,int);
#else
         va_start(argptr,nval);
#endif
         fiovfnum = nval;
         fiovfunc = getval;
         for(i = 0; i < nval; i++) vfmtval[i] = va_arg(argptr,int);
         va_end(argptr);
      }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; name of routine:  FIOVFINI -- Initialize a Variable FORTRAN Format
     ;
     ; This function initializes the variables format strings for use in the FORMAT
     ; environment needed by the FORTRAN style input/output statements. This
     ; function is needed to convert the format string into external display code
     ; which is used by the format system.
     ;
     ; calling parameters:
     ;
     ; return parameters:
     ;
     ; A zero if the format appears well-formed, else an error code. See the general
     ; discussion if FORTRAN I/O capabilities for a listing of the possible error
     ; codes.
     ;
     ; useful notes and assumptions:
     ;
     ; Element       Description of use
     ; -------       ------------------
     ; FORTRAN       READ or WRITE statements with variable format
     ;
     ; If the current conventions support separate internal and external display
     ; modes, convert the sting to external form and save its start and length
     ; so that it may be reconverted at the conclusion of its processing.
     ;
     ; initial author: (FKG) Fred Goodman, Promula Development Corporation
     ;
     ; creation date: 03/14/86 as part of Promula.Fortran Version 1.00
     ;
     ;/doc/ ************************************************************************
   */

#ifdef FPROTOTYPE
int fiovfini(char** fmt,int /* nfmt */)
#else
      int fiovfini(fmt,nfmt)
      char** fmt;                    /* Pointer to the format */
   int nfmt;                      /* Length of FORMAT */
#endif
   {
      // nfmt;
      return fiofini(fmt,0);
   }

   /*
     ;/hdr/ ************************************************************************
     ;
     ; FifBits.c: Fortran Runtime Bit Manipulation
     ;
     ;/hdr/ ************************************************************************
   */
   // #include "platform.h"             /* Define the platform hosting this code */
   // #include "pfclib.h"               /* Define PROMULA.FORTRAN library symbols */
   /*
     ;/doc/ ***********************************************************************
     ;
     ; fifbtest: FORTRAN Intrinsic Function BTEST
     ;
     ; Synopsis of Service:
     ;
     ; #include "fortran.h"                 pFortran runtime library interface
     ;
     ; int fifbtest                         FORTRAN Intrinsic function BTEST
     ; (
     ;    ULONG bits                        Bit vector whose bit is to be tested
     ;    LONG  ibit                        Number of bit to test
     ; )
     ;
     ; Description of Service:
     ;
     ; This service extracts a specified bit value from a long bit string.
     ;
     ; Properties of Service:
     ;
     ; bits         Bit vector whose bit is to be tested
     ;
     ; ibit         Number of bit to test
     ;
     ; Return Value from Service:
     ;
     ; The service returns a one or a zero depending upon whether the bit specified
     ; is on or off.
     ;
     ;/doc/ ************************************************************************
   */
#ifdef FPROTOTYPE
   int fifbtest(ULONG bits,LONG ibit)
#else
      int fifbtest(bits,ibit)
      ULONG bits;                       /* Bit vector use bit is to be tested */
   LONG ibit;                        /* Number of bit to test */
#endif
   {
      return ((1 << ibit) & bits) ? 1 : 0;
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; fifi2bits: FORTRAN Intrinsic Function I2BITS
     ;
     ; Synopsis of Service:
     ;
     ; #include "fortran.h"                 pFortran runtime library interface
     ;
     ; short fifi2bits                      FORTRAN Intrinsic function I2BITS
     ; (
     ;    short bits                        Bit vector with bits to be extracted
     ;    LONG  ibit                        First bit to move 
     ;    LONG  len                         Number of bits to move
     ; )
     ;
     ; Description of Service:
     ;
     ; This service extracts the specified number of bits from the specified vector.
     ;
     ; Properties of Service:
     ;
     ; bits         Bit vector with bits to be extracted
     ;
     ; ibit         First bit to move
     ;
     ; len          Number of bits to move
     ;
     ; Return Value from Service:
     ;
     ; The service returns the value of the extracted bits.
     ;
     ;/doc/ ************************************************************************
   */
#ifdef FPROTOTYPE
   short fifi2bits(short bits,LONG ibit,LONG len)
#else
      short fifi2bits(bits,ibit,len)
      short bits;                    /* Bit vector with bit is to be cleared */
   LONG ibit;                     /* First bit to move */
   LONG len;                      /* Number of bits to move */
#endif
   {
      short n;
      int j;
      int ib;

      n = 0;
      j = 0;
      for(ib = 0; ib < len; ib++, j++, ibit++) 
         if((1 << ibit) & bits) n |= (1 << j);
      return n;
   }

   /*
     ;/doc/ ***********************************************************************
     ;
     ; fifi2bset: FORTRAN Intrinsic Function IBSET2
     ;
     ; Synopsis of Service:
     ;
     ; #include "fortran.h"                 pFortran runtime library interface
     ;
     ; short fifi2bset                      FORTRAN intrinsic function IBSET2
     ; (
     ;    short bits                        Bit vector whose bit is to be set
     ;    short ibit                        Number of bit to set
     ; )
     ;
     ; Description of Service:
     ;
     ; This service sets a specified bit value in a short bit string.
     ;
     ; Properties of Service:
     ;
     ; bits         Bit vector whose bit is to be set
     ;
     ; ibit         Number of bit to set
     ;
     ; Return Value from Service:
     ;
     ; The service returns the value of short bit vector after the specified bit has
     ; been set.
     ;
     ;/doc/ ************************************************************************
   */
#ifdef FPROTOTYPE
   short fifi2bset(short bits,short ibit)
#else
      short fifi2bset(bits,ibit)
      short bits;                    /* Bit vector whose bit is to be set */
short ibit;                    /* Number of bit to set */
#endif
{
   return (short)(bits | (1 << ibit));
}

/*
;/doc/ ***********************************************************************
;
; fifi2shf: FORTRAN Intrinsic Function I2SHFT
;
; Synopsis of Service:
;
; #include "fortran.h"                 pFortran runtime library interface
;
; short fifi2shf                       FORTRAN intrinsic function I2SHFT
; (
;    short a                           Value to be shifted
;    int   n                           Number of positions to be shifted
; )
;
; Description of Service:
;
; This service shifts the short argument left or right the specified number of
; bit positions depending upon whether the position count is positive or
; negative. The approach determines if the shift count is positive or negative
; and then performs the specified shift and return the result.
;
; Properties of Service:
;
; a            Value to be shifted
;
; n            Number of positions to be shifted
;
; Return Value from Service:
;
; The short integer result.
;
;/doc/ ************************************************************************
*/
#ifdef FPROTOTYPE
short fifi2shf(short a,int n)
#else
short fifi2shf(a,n)
short a;                       /* Value to be shifted */
int n;                         /* Number of positions to be shifted */
#endif
{
   if(n < 0) a = (short) (a >> (-n));
   else if(n > 0) a = (short) (a << n);
   return a;
}

/*
;/doc/ ***********************************************************************
;
; fifibclr: FORTRAN Intrinsic Function IBCLR
;
; Synopsis of Service:
;
; #include "fortran.h"                 pFortran runtime library interface
;
; ULONG fifibclr                       FORTRAN intrinsic function IBCLR
; (
;    ULONG bits                        Bit vector with bit to be cleared
;    LONG  ibit                        Number of bit to clear
; )
;
; Description of Service:
;
; This service clears a specified bit value in a long bit string. The approach
; just clears the bit and returns the new value.
;
; Properties of Service:
;
; bits         Bit vector with bit is to be cleared
;
; ibit         Number of bit to clear
;
; Return Value from Service:
;
; The service returns the value of the long bit mask after the specified bit
; has been cleared.
;
;/doc/ ************************************************************************
*/
#ifdef FPROTOTYPE
ULONG fifibclr(ULONG bits,LONG ibit)
#else
ULONG fifibclr(bits,ibit)
ULONG bits;                    /* Bit vector with bit is to be cleared */
LONG ibit;                     /* Number of bit to clear */
#endif
{
   return (bits & ~(1 << ibit));
}

/*
;/doc/ ***********************************************************************
;
; fifibit: FORTRAN Intrinsic Function IBIT
;
; Synopsis of Service:
;
; #include "fortran.h"                 pFortran runtime library interface
;
; void fifibit                         FORTRAN intrinsic function IBIT
; (
;    UBYTE* bits                       Bit vector to receive new bit value
;    int    ibit                       Number of bit to receive value
;    int    ival                       Bit value to be inserted
; )
;
; Description of Service:
;
; This service sets a specified bit in an arbitrary bit string equal to a
; specified value. The left-most bit position is numbered 1, with higher bits
; receiving higher values.
;
; Properties of Service:
;
; bits         Bit vector to receive new bit value
;
; ibit         Number of bit to receive value
;
; ival         Bit value to be inserted
;
; Return Value from Service: None
;
;/doc/ ************************************************************************
*/
#ifdef FPROTOTYPE
void fifibit(UBYTE* bits,int ibit,int ival)
#else
void fifibit(bits,ibit,ival)
UBYTE* bits;                   /* Bit vector to receive new bit value */
int ibit;                      /* Number of bit to receive value */
int ival;                      /* Bit value to be inserted */
#endif
{
   static UBYTE mask[8] = {128,64,32,16,8,4,2,1};
   int ipos;
   int ibyte;

   /*--------------------------------------------------------------------------
   ;
   ; Step 1: Compute the number of the byte containing the bit and the number
   ; of the bit within that byte. Both byte and bit numbers start at zero for
   ; the left most bit.
   ;
   ;-------------------------------------------------------------------------*/

   ibit--;
   ipos = ibit % 8;
   ibyte = ibit/8;

   /*--------------------------------------------------------------------------
   ;
   ; Step 2: Set the specified on using a mask and then if the specified value
   ; is zero, XOR the bit to zero using the same mask.
   ;
   ;-------------------------------------------------------------------------*/

   bits[ibyte] |= mask[ipos]; 
   if(ival == 0) bits[ibyte] ^= mask[ipos];
}

/*
;/doc/ ***********************************************************************
;
; fifibits: FORTRAN Intrinsic Function IBITS
;
; Synopsis of Service:
;
; #include "fortran.h"                 pFortran runtime library interface
;
; ULONG fifibits                       FORTRAN intrinsic function IBITS
; (
;    ULONG bits                        Bit vector with bits to be extracted
;    LONG  ibit                        First bit to extract
;    LONG  len                         Number of bits to extract
; )
;
; Description of Service:
;
; This service extracts the specified number of bits from the specified vector.
;
; Properties of Service:
;
; bits         Bit vector with bits to be extracted
;
; ibit         First bit to extract
;
; len          Number of bits to extract
;
; Return Value from Service:
;
; The service returns the value of the extracted bits.
;
;/doc/ ************************************************************************
*/
#ifdef FPROTOTYPE
ULONG fifibits(ULONG bits,LONG ibit,LONG len)
#else
ULONG fifibits(bits,ibit,len)
ULONG bits;                    /* Bit vector with bits to be extracted */
LONG ibit;                     /* First bit to extract */
LONG len;                      /* Number of bits to extract */
#endif
{
   unsigned int n;
   int j;
   int ib;

   n = 0;
   j = 0;
   for(ib = 0; ib < len; ib++, j++, ibit++) 
   if((1 << ibit) & bits) n |= (1 << j);
   return n;
}

/*
;/doc/ ***********************************************************************
;
; fifibset: FORTRAN Intrinsic Function IBSET
;
; Synopsis of Service:
;
; #include "fortran.h"                 pFortran runtime library interface
;
; ULONG fifibset                       FORTRAN Intrinsic function IBSET
; (
;    ULONG bits                        Bit vector with bit is to be set
;    LONG  ibit                        Number of bit to set
; )
;
; Description of Service:
;
; This service sets a specified bit value in a long bit string.
;
; Properties of Service:
;
; bits         Bit vector with bit is to be set
;
; ibit         Number of bit to set
;
; Return Value from Service:
;
; The service returns the value of the long bit vector after the specified bit
; has been set.
;
;/doc/ ************************************************************************
*/
#ifdef FPROTOTYPE
ULONG fifibset(ULONG bits,LONG ibit)
#else
ULONG fifibset(bits,ibit)
ULONG bits;                    /* Bit vector with bit is to be set */
LONG ibit;                     /* Number of bit to set */
#endif
{
   return (bits | (1 << ibit));
}

/*
;/doc/ ***********************************************************************
;
; fifishf: FORTRAN Intrinsic Function ISHFT
;
; Synopsis of Service:
;
; #include "fortran.h"                 pFortran runtime library interface
;
; LONG fifishf                         FORTRAN intrinsic function ISHFT
; (
;    LONG a                            Value to be shifted
;    int  n                            Number of positions to be shifted
; )
;
; Description of Service:
;
; This service shifts the long argument left or right the specified number of
; bit positions depending upon whether the position count is positive or
; negative. The approach determines if the shift count and positive or negative
; and then performs the specified shift and returns the result.
;
; Properties of Service:
;
; a            Value to be shifted
;
; n            Number of positions to be shifted
;
; Return Value from Service:
;
; The long integer result.
;
;/doc/ ************************************************************************
*/
#ifdef FPROTOTYPE
LONG fifishf(LONG a,int n)
#else
LONG fifishf(a,n)
LONG a;                        /* Value to be shifted */
int n;                         /* Number of positions to be shifted */
#endif
{
   if(n < 0) a = a >> (-n);
   else if(n > 0) a = a << n;
   return a;
}

/*
;/doc/ ***********************************************************************
;
; fifmvbit: FORTRAN Intrinsic Function MVBITS
;
; Synopsis of Service:
;
; #include "fortran.h"                 pFortran runtime library interface
;
; void fifmvbit                        FORTRAN Intrinsic function MVBITS
; (
;    ULONG  m                          Source of bits to move
;    LONG   i                          Position of first bit to move
;    LONG   len                        Number of bits to move
;    ULONG* n                          Destination of bits
;    LONG   j                          Position of first bit in destination
; )
;
; Description of Service:
;
; This service moves a specified number of bits from a source word to a
; destination word.
;
; Properties of Service:
;
; m            Source of bits to move
;
; i            Position of first bit to move
;
; len          Number of bits to move
;
; n            Destination of bits
;
; j            Position of first bit in destination
;
; Return Value from Service: None
;
;/doc/ ************************************************************************
*/
#ifdef FPROTOTYPE
void fifmvbit(ULONG m,LONG i,LONG len,LONG* n,LONG j)
#else
void fifmvbit(m,i,len,n,j)
ULONG m;                       /* Source of bits to move */
LONG i;                        /* Position of first bit to move */
LONG len;                      /* Number of bits to move */
ULONG* n;                      /* Destination of bits */
LONG j;                        /* Position of first bit in destination */
#endif
{
   int ib;                   /* Dummy bit counter */
   ULONG bits;               /* Local copy of return value */

   bits = 0;
   for(ib = 0; ib < len; ib++,i++,j++) 
   {
      if((1 << i) & m) bits |= (1 << j);
      else bits &= ~(1 << j);
   }
   *n = bits;
}

/*
;/doc/ ***********************************************************************
;
; fifrbit: FORTRAN Intrinsic Function RBIT
;
; Synopsis of Service:
;
; #include "fortran.h"                 pFortran runtime library interface
;
; void fifrbit                         FORTRAN intrinsic function RBIT
; (
;    UBYTE* bits                       Bit vector to be reversed
;    int    nbyte                      Number of bytes in bit vector
; )
;
; Description of Service:
;
; This service reverses the order of the bytes in a bit string.
;
; Properties of Service:
;
; bits         Bit vector to be reversed
;
; nbyte        Number of bytes in bit vector
;
; Return Value from Service: None
;
;/doc/ ************************************************************************
*/
#ifdef FPROTOTYPE
void fifrbit(UBYTE* bits,int nbyte)
#else
void fifrbit(bits,nbyte)
UBYTE* bits;                   /* Bit vector to be reversed */
int nbyte;                     /* Number of bytes in bit vector */
#endif
{
   int c;
   int i;
   int j;
    
   for(i = 0, j = nbyte-1; i < j; i++, j--) 
   {
      c = *(bits+i);
      *(bits+i) = *(bits+j);
      *(bits+j) = (UBYTE) c;
   }
}

/*
;/doc/ ***********************************************************************
;
; fifxbit: FORTRAN Intrinsic Function XBIT
;
; Synopsis of Service:
;
; #include "fortran.h"                 pFortran runtime library interface
;
; int fifxbit                          FORTRAN intrinsic function XBIT
; (
;    UBYTE* bits                       Vector containing bit to be extracted
;    int    ibit                       Number of bit to be extracted
; )
;
; Description of Service:
;
; This service extracts a specified bit value from an arbitrary bit string.
; The left-most bit position is numbered 1, with higher bits receiving higher
; values.
;
; Properties of Service:
;
; bits         Bit vector containing bit to be extracted
;
; ibit         Number of bit to be extracted
;
; Return Value from Service:
;
; The service returns a one or a zero depending upon whether the bit specified
; is on or off.
;
;/doc/ ************************************************************************
*/
#ifdef FPROTOTYPE
int fifxbit(UBYTE* bits,int ibit)
#else
int fifxbit(bits,ibit)
UBYTE* bits;                   /* Bit vector containing bit to be extracted */
int ibit;                      /* Number of bit to be extracted */
#endif
{
   static UBYTE mask[8] = {128,64,32,16,8,4,2,1};
   int ipos;
   int ibyte;

   /*--------------------------------------------------------------------------
   ;
   ; Step 1: Compute the number of the byte containing the bit and the
   ; number of the bit within that byte. Both byte and bit numbers start at
   ; zero for the left most bit.
   ;
   ;-------------------------------------------------------------------------*/

   ibit--;
   ipos = ibit % 8;
   ibyte = ibit/8;

   /*--------------------------------------------------------------------------
   ;
   ; Step 2: Test the specified bit and return the result.
   ;
   ;--------------------------------------------------------------------------*/

   return ((bits[ibyte] & mask[ipos]) != 0); 
}

/*
;/doc/ ***********************************************************************
;
; fifxcrep: FORTRAN Extended Character Representation
;
; Synopsis of Service:
;
; #include "fortran.h"                 pFortran runtime library interface
;
; char* fifxcrep                       FORTRAN extend character representation
; (
;    char* chrs                        Vector of characters
; )
;
; Description of Service:
;
; In several dialects of FORTRAN, especially Prime FORTRAN 77, the normal
; internal character representation is a seven bit code with the 8th bit
; always set on, while the extended characters have the 8th bit off. The
; normal external character representions use the opposite conventions.
; This service converts string constants to the extended representation
; to make them compatible with other "internal" characters. Generally, if the
; high bit of the first character is already set, then this string has
; already been converted; else move through each character and set the high
; bit.
;
; Properties of Service:
;
; chrs         Vector of characters
;
; Return Value from Service:
;
; A pointer to the result string.
;
;/doc/ ************************************************************************
*/
#ifdef FPROTOTYPE
char* fifxcrep(char* chrs)
#else
char* fifxcrep(chrs)
char* chrs;                    /* Vector of characters */
#endif
{
   UBYTE* s;

   s = (UBYTE*) chrs;
   if(!(*s & 0x80)) while(*s) *s++ |= 0x80;
   return chrs;
}


/*
;/hdr/ ************************************************************************
;
; Copyright 1986-1998 Promula Development Corporation ALL RIGHTS RESERVED
;
; name of module: FioOper.c -- Fortran I/O, String Conversions
;
; global symbols defined:
;
; useful notes and assumptions:
;
; initial author: (FKG) Fred Goodman, Promula Development Corporation
;
; edit 06/17/98 FKG: Changed names of global symbols to conform to the 
;                    Promula version 7.08 API naming conventions. 
;
; creation date: 03/14/86 as part of Promula.Fortran Version 1.00
;
;/hdr/ ************************************************************************
*/
#define CHARTYPE                  /* Gives access to character type info */
#define PRIOFUNC                  /* Gives access to PROMULA I/O functions */
#define MATHFUNC                  /* Gives access to real math functions */
#define RAWMFUNC                  /* Gives access to raw memory functions */
#define STRGFUNC                  /* Gives access to the string functions */
// #include "platform.h"             /* Define the platform hosting this code */
// #include "pfclib.h"               /* Define PROMULA.FORTRAN library symbols */
// #include "chrapi.h"               /* Define character manipulation interface */

#ifdef FPROTOTYPE
void fifibit(unsigned char* bits,int ibit,int ival);
void fifrbit(unsigned char* bits,int nbyte);
int  fifxbit(unsigned char* bits,int ibit);
int  FioPicture(double value,char* Picture,char* crec);
void fiowtxt(void);
#else
extern void fifibit();            /* Insert a bit */
extern void fifrbit();            /* Reverse bits byte order */
extern int  fifxbit();            /* Extract a bit */
extern int  FioPicture();
extern void fiowtxt();            /* Write text record */
#endif

extern char fiobfmt[40];          /* Business formatting string */
extern int  fioblkn;              /* Blanks are null convention */
extern int  fiobsex;              /* Is byte sex little-end first */
extern int  fioconv;              /* General dialect convention flags */
extern char fiocrec[FIOBMAX];     /* Coded communications record */
extern int  fioefc;               /* E-format exponent character */
extern int  fioefw;               /* E-format exponent width */
extern int  fioerc;               /* Position of end of conversion */
extern int  fioffw;               /* Record width for free-form output */
extern int  fiofspec;             /* Current format specification */
extern int  fioier;               /* Code for actual error encountered */
extern int  fioiwd;               /* Field width */
extern int  fionchar;             /* Number of output characters */
extern int  fiondec;              /* Minimum digit count */
extern int fioscal;               /* Floating point scale factor */

/*****************************************************************************/
/*                                                                           */
/* FIOWHEXO: Write Hexadecimal or Octal Constant                             */
/*                                                                           */
/* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
/*                                                                           */
/* Author: Fred Goodman                                                      */
/*                                                                           */
/* Purpose:                                                                  */
/*                                                                           */
/* Displays a numeric value as a hexidecimal or octal character string.      */
/*                                                                           */
/* Return value:                                                             */
/*                                                                           */
/* The function returns a pointer to the exact representation string for     */
/* the value.                                                                */
/*                                                                           */
/* See also:                                                                 */
/*                                                                           */
/* Process:                                                                  */
/*                                                                           */
/* See detailed steps below.                                                 */
/*                                                                           */
/*****************************************************************************/

#ifdef FPROTOTYPE
char* fiowhexo(int base,char* ival,int vlen)
#else
char* fiowhexo(base,ival,vlen)
int base;                         /* Base of exact representation constant */
char* ival;                       /* Value of exact representation constant */
int vlen;                         /* Length of value in bytes */
#endif
{
   static char xcons[32];
   int i;
   int minbit;
   unsigned char ic;
   int j;
   int vbit;
   int xlen;

   /*------------------------------------------------------------------------*/
   /*                                                                        */
   /* Step 1: If the system has "little-end-first" byte sex, reverse         */
   /* the order of the bytes.                                                */
   /*                                                                        */
   /*------------------------------------------------------------------------*/

   if(fiobsex) fifrbit((unsigned char*)ival,vlen);

   /*------------------------------------------------------------------------*/
   /*                                                                        */
   /* Step 2: Compute the number of bits in the resultant exact representa-  */
   /* tion string, the length of the exact representation string, and the    */
   /* minimumn bit number of the digit value.                                */
   /*                                                                        */
   /*------------------------------------------------------------------------*/

   vbit = vlen * 8;
   if(base == 8)
   {
      minbit = 5;
      xlen = (vbit - 1) / 3 + 1;
   }
   else
   {
      minbit = 4;
      xlen = vlen * 2;
   }

   /*------------------------------------------------------------------------*/
   /*                                                                        */
   /* Step 3: Move through the digits from the back to the front and place   */
   /* the exact representation display in the result string.                 */
   /*                                                                        */
   /*------------------------------------------------------------------------*/

   for(i = xlen-1; i >= 0 && vbit >= 0; i--)
   {
      ic = 0;
      for(j = 8; j > minbit; j--)
      {
         fifibit(&ic,j,fifxbit((unsigned char*)ival,vbit));
         vbit--;
         if(vbit == 0) break;
      }
      if(ic < 10) ic = ChrDigits[ic];
      else ic = ChrAlphabet[ic-10];
      xcons[i] = ic;
   }
   xcons[xlen] = '\0';

   /*------------------------------------------------------------------------*/
   /*                                                                        */
   /* Step 4: If the system has little-end-first byte sex, restore the order */
   /* of the bytes to their original order.                                  */
   /*                                                                        */
   /*------------------------------------------------------------------------*/

   if(fiobsex) fifrbit((unsigned char*)ival,vlen);
   return xcons;
}

/*****************************************************************************/
/*                                                                           */
/* FIOSTOD: Convert String to Double                                         */
/*                                                                           */
/* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
/*                                                                           */
/* Author: Fred Goodman                                                      */
/*                                                                           */
/* Purpose:                                                                  */
/*                                                                           */
/* Converts an alphanumeric string containing a number in scientific         */
/* notation to a double precision floating point number. The string can      */
/* contain optional leading blanks, an integer part, a fractional part, and  */
/* an exponent part. The integer part consists of an optional sign followed  */
/* by zero or more decimal digits. The fractional part is a decimal point    */
/* followed by zero or more decimal digits. The exponent part consists of    */
/* an 'E', 'e', 'D', or 'd' followed by an optional sign and a sequence of   */
/* decimal digits. The parameters to this function are as follows:           */
/*                                                                           */
/* Name    Description of Use                                                */
/* ----    ------------------                                                */
/* str     Contains the alpanumeric string to be converted.                  */
/* nstr    Contains the number of characters in the string. Note             */
/*         that the string is not necessarily NULL terminated.               */
/*                                                                           */
/* The following global variables are also used by this function:            */
/*                                                                           */
/* Name    Description of Use                                                */
/* ----    ------------------                                                */
/* fioerc  If the  conversion  encounters  a character  which is not part of */
/*         the notation then this variable returns the position of that      */
/*         character.                                                        */
/* fiondec Specifies the number of implied decimal places in the value.      */
/* fioblkn If blanks are normal tie-breakers,  then this variable is zero,   */
/*         else if it is +1 blanks are simply ignored and if it is -1 blanks */
/*         are treated as zero.                                              */
/*                                                                           */
/* Return value:                                                             */
/*                                                                           */
/* The double precision value of the string as computed.                     */
/*                                                                           */
/* See also:                                                                 */
/*                                                                           */
/* Referenced by:                                                            */
/*                                                                           */
/* Element       Description of use                                          */
/* -------       ------------------                                          */
/* fiordi        Convert short integer string to a value                     */
/* fiordi        Convert long integer string to a value                      */
/* fiordd        Convert double precision string to a value                  */
/* fiordf        Convert a single precision string to a value                */
/*                                                                           */
/* Process:                                                                  */
/*                                                                           */
/* See detailed steps below.                                                 */
/*                                                                           */
/*****************************************************************************/

#ifdef FPROTOTYPE
double fiostod(char *str,int nstr)
#else
double fiostod(str,nstr)
char *str;                        /* String to be converted */
int nstr;                         /* Length of field */
#endif
{    
   double dval;              /* Return value */
   int expon;                /* Power of ten to multiply times value */
   char* sadr;               /* Pointer to start of value string */
   char* lstr;               /* Temporary string pointer */
   int sign;                 /* True if there was a minus, else false */
   int ndig;                 /* Number of numeric digits in string */
   int idig;                 /* Value of digit beyond end of string */
   int ndec;                 /* Number of decimal places */

   /*------------------------------------------------------------------------*/
   /*                                                                        */
   /* Step 1: Initialize the needed control variables. Note that the string  */
   /* is temporarily made to be NULL terminated to simplify the logic. Note  */
   /* also that if the string string is empty then a zero is simply returned.*/
   /*                                                                        */
   /*------------------------------------------------------------------------*/

   ChrFlags |= CHR_HASEXPON;
   ChrFlags ^= CHR_HASEXPON;
   fioerc = ndec = 0;
   dval = 0.0;
   if(!nstr) goto fiostod8;
   sign = expon = ndig = 0;
   sadr = str;
   idig = *(str + nstr);
   *(str + nstr) = '\0';

   /*------------------------------------------------------------------------*/
   /*                                                                        */
   /* Step 2: Skip over any leading blanks.                                  */
   /*                                                                        */
   /*------------------------------------------------------------------------*/

   while(*str == ' ' || *str == '\t') str++;

   /*------------------------------------------------------------------------*/
   /*                                                                        */
   /* Step 3: If there is a leading sign, set the sign indicator accordingly.*/
   /*                                                                        */
   /*------------------------------------------------------------------------*/

   if(*str == '-' || *str == '+')  if(*str++ == '-') sign++;

   /*------------------------------------------------------------------------*/
   /*                                                                        */
   /* Step 4: Calculate the value of the string and the number of decimal    */
   /* places.                                                                */
   /*                                                                        */
   /*------------------------------------------------------------------------*/

fiostod4:
   while (ChrTypeFlags[(int)*str] & CNUMBER)
   {
      ndig++;
      dval *= 10.0;
      if(ndig < 18) dval += (int) ChrInformation[(int)*str];  
      str++;
      if(ndec) ndec++;
   }
   if((*str == ' ' || *str == '\t') && fioblkn)
   {
      str++;
      if(fioblkn < 0)
      {
         ndig++;
         dval *= 10.0;
         if(ndec) ndec++;
      }
      goto fiostod4;
   }
   if (*str == '.' && !ndec)
   {
      str++;
      ndec++;
      goto fiostod4;
   }
   if(ndec) expon = 1-ndec;

   /*------------------------------------------------------------------------*/
   /*                                                                        */
   /* Step 5: If there is an exponent specified, calculate its value.        */
   /*                                                                        */
   /*------------------------------------------------------------------------*/

   if(*str == 'e' || *str == 'E' || *str == 'd' || *str == 'D' ||
                           *str == '+' || *str == '-')
   {
      ChrFlags |= CHR_HASEXPON;
      lstr = str;
      if(*str != '+' && *str != '-') str++;
      while(*str == ' ' || *str == '\t') str++;
      if(*str == '-' || *str == '+' || (ChrTypeFlags[(int)*str] & CNUMBER))
      {
         expon += ChrShortFromString(&str,fioblkn);
      }
      else if(*str != '\0') str = lstr;
   }
   if(*str != '\0' && *str != ' ' && *str != ',' && *str != ')' 
                                  && *str != '/' && *str != '\t')
   {
      fioier = ESTD_NNC;
   }

   /*------------------------------------------------------------------------*/
   /*                                                                        */
   /* Step 6: Adjust the value by the appropriate power of ten and by any    */
   /* sign.                                                                  */
   /*                                                                        */
   /*------------------------------------------------------------------------*/

   if(expon) dval *= pow(10.0,(double) expon);
   if(sign) dval = -dval;

   /*------------------------------------------------------------------------*/
   /*                                                                        */
   /* Step 7: If the end of string has not been reached, set the position of */
   /* the ending character and restore the trailing digit.                   */
   /*                                                                        */
   /*------------------------------------------------------------------------*/

   if(*str) fioerc = str - sadr + 1;
   *(sadr + nstr) = (char) idig;

   /*------------------------------------------------------------------------*/
   /*                                                                        */
   /* Step 8: Adjust the current value for any implied decimal places and    */
   /* return it to the calling routine.                                      */
   /*                                                                        */
   /*------------------------------------------------------------------------*/

fiostod8:
   if(!ndec && (fiondec > 0)) dval /= pow(10.0,(double) fiondec);
   return dval;
}

/*****************************************************************************/
/*                                                                           */
/* FIOWDBL: Write Double Precision Value                                     */
/*                                                                           */
/* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
/*                                                                           */
/* Author: Fred Goodman                                                      */
/*                                                                           */
/* Purpose:                                                                  */
/*                                                                           */
/* Converts a double precision value to free-form display form and stores it */
/* at the current position in the coded communications record.               */
/*                                                                           */
/* The parameter "value" contains the value to be converted; while "nsigdig" */
/* specifies the number of significant digits in the value.                  */
/*                                                                           */
/* Return value:                                                             */
/*                                                                           */
/* None, the function is void. The global variable "fiocrec" is updated to   */
/* contain the new field, and the variable "fionchar" is updated to reflect  */
/* the new character count in the coded communication record.                */
/*                                                                           */
/* See also:                                                                 */
/*                                                                           */
/* Referenced by:                                                            */
/*                                                                           */
/* Element       Description of use                                          */
/* -------       ------------------                                          */
/* fiownl        Write namelist values                                       */
/*                                                                           */
/* Process:                                                                  */
/*                                                                           */
/* See detailed steps below.                                                 */
/*                                                                           */
/*****************************************************************************/

#ifdef FPROTOTYPE
void fiowdbl(double value,int nsigdig)
#else
void fiowdbl(value,nsigdig)
double value;                     /* Value to be converted */
int nsigdig;                      /* Number of significant digits */
#endif
{
   LONG lvalue;              /* Long equivalent of value */
   int msign;                /* Minus indicator */
   int nd;                   /* Number of decimal places in value */
   int nc;                   /* Miscellaneous character count */
   char* s;                  /* Temporary string pointer */
   char s1[30];              /* Temporary display buffer */
   char digits[8];
   double temp;              /* Intermediate value */

   /*------------------------------------------------------------------------*/
   /*                                                                        */
   /* Step 1: If the value is simply zero, then enter the appropriate        */
   /* display and end local processing.                                      */
   /*                                                                        */
   /*------------------------------------------------------------------------*/

   if(value == 0.0)
   {
      cpymem("0.0",fiocrec+fionchar,3);
      fionchar += 3;
      return;
   }

   /*------------------------------------------------------------------------*/
   /*                                                                        */
   /* Step 2: Determine if the value is has any decimal digits, and if so    */
   /* branch to step 5.                                                      */
   /*                                                                        */
   /*------------------------------------------------------------------------*/

   temp = fabs(value);
   if(temp >= 1.0 && temp < 1.0E12)
   {

      /*---------------------------------------------------------------------*/
      /*                                                                     */
      /* Step 3: The value has no decimal digits. Enter into the output      */
      /* record and end local processing.                                    */
      /*                                                                     */
      /*---------------------------------------------------------------------*/

      lvalue = (LONG) value;
      if(temp >= 1.0 && fabs((double)lvalue - value) < 1.0E-10)
      {
         if(lvalue >= 0 && lvalue < 10)
         {
            fiocrec[fionchar++] = ChrDigits[(int) lvalue];
            fiocrec[fionchar++] = '.';
            fiocrec[fionchar++] = '0';
            return;
         }
         ChrDoubleToChar(value,nsigdig,&nc,&msign,s1);
         nc += ChrRoundValue(s1,nc,nsigdig);
         *(s1+nc++) = '.';
         *(s1+nc++) = '0';
         *(s1+nc) = '\0';
         if(msign) fiocrec[fionchar++] = '-';
         cpymem(s1,fiocrec+fionchar,nc);
         fionchar += nc;
         return;
      }
   }

   /*------------------------------------------------------------------------*/
   /*                                                                        */
   /* Step 4: The value is very large or contains decimal digits. Convert    */
   /* all of its digits to display form, and then remove any trailing nines, */
   /* if the value was computationally trunctated. Finally, remove any       */
   /* trailing zeros.                                                        */
   /*                                                                        */
   /*------------------------------------------------------------------------*/

   nc = nsigdig;
   ChrDoubleToChar(value,nc,&nd,&msign,s1);
   if(*(s1+nc-1) == '9' && *(s1+nc-2) == '9')
   {
      nc -= 2;
      nd += ChrRoundValue(s1,nc,nsigdig);
   }
   while((nc > 1) && (*(s1+nc-1) == '0')) nc--;
   *(s1+nc) = '\0';

   /*------------------------------------------------------------------------*/
   /*                                                                        */
   /* Step 5: Form the remainder of the display, using E-format notation if  */
   /* necessary.                                                             */
   /*                                                                        */
   /*------------------------------------------------------------------------*/

   if(nd >= 0 && nd < nc)
   {
      ChrShiftRight(s1+nd,1,'.');
      nc++;
      if(nd == 0)
      {
         ChrShiftRight(s1,1,'0');
         nc++;
      }
   }
   else if(nd >= nc && nd < (nc + 3))
   {
      while(nc < nd) *(s1+nc++) = '0';
      *(s1+nc++) = '.';
      *(s1+nc++) = '0';
      *(s1+nc) = '\0';
   }
   else
   {
      if(nc == 1)
      {
         nc = 2;
         *(s1+1) = '0';
         *(s1+2) = '\0';
      }
      ChrShiftRight(s1+1,1,'.');
      nc++;
      nd--;
      *(s1+nc++) = 'E';
      ChrShortString(nd,digits,0);
      s = digits;
      while(*s) *(s1 + nc++) = *s++;
   }
   if(msign) fiocrec[fionchar++] = '-';
   cpymem(s1,fiocrec+fionchar,nc);
   fionchar += nc;
}

/*****************************************************************************/
/*                                                                           */
/* FIOLTOS: Convert Long Integer to String                                   */
/*                                                                           */
/* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
/*                                                                           */
/* Author: Fred Goodman                                                      */
/*                                                                           */
/* Purpose:                                                                  */
/*                                                                           */
/* Converts a long integer value to display form and stores it at the        */
/* current position in the coded communications record right-justified in a  */
/* fixed length field. In particular, the output field consists of blanks,   */
/* if necessary, followed by a minus sign if the internal value is negative, */ 
/* or an optional plus sign otherwise. If the number of signifcant digits    */
/* and sign required to represent the value is less than the specified width,*/
/* the unused leftmost portion of the field is filled with blanks. If it is  */
/* greater than the width, asterisks are entered instead of numeric digits.  */
/* If a minimum digit count is specified, then the output field consists of  */
/* at least that many digits, and is zero-filled as necessary. If the        */
/* minimum digit count is zero, and the value is zero, then the field is     */
/* simply blank filled, regardless of any sign control in effect.            */
/*                                                                           */
/* The parameter "value" contains the value to be converted. The conversion  */
/* control parameters are specified via global variables as shown in "See    */
/* also".                                                                    */
/*                                                                           */
/* Return value:                                                             */
/*                                                                           */
/* None, the function is void. The global variable "fiocrec" is updated to   */
/* contain the new field, and the variable "fionchar" is updated to reflect  */
/* the new character count if the coded communication record.                */
/*                                                                           */
/* See also:                                                                 */
/*                                                                           */
/* Referenced by:                                                            */
/*                                                                           */
/* Element       Description of use                                          */
/* -------       ------------------                                          */
/* fiowri        Convert a single short value                                */
/* fiowrl        Convert a single long value                                 */
/*                                                                           */
/* Process:                                                                  */
/*                                                                           */
/* See detailed steps below.                                                 */
/*                                                                           */
/*****************************************************************************/

#ifdef FPROTOTYPE
void fioltos(LONG value)
#else
void fioltos(value)
LONG value;                    /* Value to be converted */
#endif
{
   int minus;                /* Minus indicator */
   int nc;                   /* Current field character count*/
   char* field;              /* Points to start of field */
   char* s;                  /* Temporary string pointer */
   char digits[8];

   /**************************************************************************/
   /*                                                                        */
   /* Step 1: If the value is zero and the minimum digit count is zero, then */
   /* simply blank fill the field and branch to step 6 to end local          */
   /* processing.                                                            */
   /*                                                                        */
   /**************************************************************************/

   field = fiocrec + fionchar;
   if(!value)
   {
      if(!fiondec && fioiwd)
      {
         filmem(field,fioiwd,' ');
         goto ltos06;
       }

      /***********************************************************************/
      /*                                                                     */
      /* Step 2: If the value is zero, but the minimum digit count is        */
      /* nonzero or unspecified, then enter a single '0' into the            */
      /* communications record and branch to step 4, to edit the display.    */
      /*                                                                     */
      /***********************************************************************/

      minus = 0;
      nc = 1;
      fiocrec[fionchar] = '0';
      fiocrec[fionchar+1] = '\0';
      goto ltos04;
   }

   /**************************************************************************/
   /*                                                                        */
   /* Step 3: The value is not zero, using C system routines convert it to   */
   /* display form. Note that all sign logic is performed locally.           */
   /*                                                                        */
   /**************************************************************************/

   if(value < 0)
   {
      value = -value;
      minus = 1;
   }
   else minus = 0;
   nc = ChrShortString(value,digits,0);
   s = digits;
   cpymem(s,field,nc+1);

   /**************************************************************************/
   /*                                                                        */
   /* Step 4: The communications record now contains the raw digits of the   */
   /* value. Edit the display to include any leading zeros and to include    */
   /* any leading sign.                                                      */
   /*                                                                        */
   /**************************************************************************/

ltos04:
   if(nc < fiondec)
   {
      ChrShiftRight(field,fiondec-nc,'0');
      nc = fiondec;
   }
   if(minus)
   {
      ChrShiftRight(field,1,'-');
      nc++;
   }
   else if(ChrFlags & CHR_PLUSSIGN)
   {
      ChrShiftRight(field,1,'+');
      nc++;
   }

   /**************************************************************************/
   /*                                                                        */
   /* Step 5: The digit display is now left-justified in the field. If it    */
   /* has overflowed, fill it with asterisks, else insert sufficient blanks  */
   /* to pad it to the desired field width.                                  */
   /*                                                                        */
   /**************************************************************************/

   if(fioiwd == 0) fioiwd = nc + 1;
   if(nc > fioiwd) filmem(field,nc,'*');
   else if(nc < fioiwd) ChrShiftRight(field,fioiwd-nc,' ');

   /**************************************************************************/
   /*                                                                        */
   /* Step 6: The display is completed. Update the number of characters in   */
   /* the communications record and end local processing.                    */
   /*                                                                        */
   /**************************************************************************/

ltos06:
   fionchar += fioiwd;
}

/*****************************************************************************/
/*                                                                           */
/* FIOWVAL: Write Floating Point Value                                       */
/*                                                                           */
/* Copyright 1988-97 PROMULA Development Corporation ALL RIGHTS RESERVED     */
/*                                                                           */
/* Author: Fred Goodman                                                      */
/*                                                                           */
/* Purpose:                                                                  */
/*                                                                           */
/* This utility function controls the conversion of floating point values to */
/* string form under the control of a FORMAT specification. The actual form  */
/* of the output display depends upon the paricular specification. For all   */
/* formats the output field consists of blanks, if necessary, followed by a  */
/* minus sign if the value is negative, or an optional plus sign otherwise.  */
/*                                                                           */
/* For F format this is followed by a string of digits that contains a       */
/* decimal point, representing the magnitude of the value. Leading zeros are */
/* not provided, except for an optional zero immediately to the left of the  */
/* decimal point if the magnitude of the value is less than one. The leading */
/* zero also appears if there would otherwise be no digits in the output     */
/* field.                                                                    */
/*                                                                           */
/* For E format this is followed by a zero, a decimal point, the number of   */
/* significant digits specified and exponent of a specified width.           */
/*                                                                           */
/* For G format, the display type demends upon whether or not all            */
/* significant digits can be displayed in F format. If they can, then F      */
/* format is used, if they cannot E format is used.                          */
/*                                                                           */
/* For B format, business formatting conventions are used. See function      */
/* "FioPicture" for details.                                                   */
/*                                                                           */
/* For all display types, if the number of digits required to represent the  */
/* value is less than the specified width, the unused leftmost portion of    */
/* the field is filled with blanks. If it is greater than the width,         */
/* asterisks are entered instead of the representation.                      */ 
/*                                                                           */
/* Return value:                                                             */
/*                                                                           */
/* None, the function is void. The global variable "fiocrec" is updated to   */
/* contain the new field, and the variable "fionchar" is updated to reflect  */
/* the new character count in the coded communications record.               */
/*                                                                           */
/* See also:                                                                 */
/*                                                                           */
/* Referenced by:                                                            */
/*                                                                           */
/* Element       Description of use                                          */
/* -------       ------------------                                          */
/* fiowrf        Convert a single precision value                            */
/* fiowrd        Conver a double precision value                             */
/*                                                                           */
/* Process:                                                                  */
/*                                                                           */
/* See detailed steps below.                                                 */
/*                                                                           */
/*****************************************************************************/

#ifdef FPROTOTYPE
void fiowval(double value)
#else
void fiowval(value)
double value;                     /* Value to be converted */
#endif
{
   char dspdig[400];         /* Temporary display buffer */
   char* field;              /* Points to start of field */
   int i;                    /* Dummy counter */
   int minus;                /* Minus indicator */
   int nc;                   /* Current field character count*/
   int nd;                   /* Position of decimal point */
   int ndec;                 /* Number of decimal places */
   int scalf;                /* Scale factor */
   int freeform;             /* Free form output is desired */

   /*------------------------------------------------------------------------*/
   /*                                                                        */
   /* Step 1: Branch according to the type of conversion that has been       */
   /* requested.                                                             */
   /*                                                                        */
   /*         Step  Specification    Code                                    */
   /*         ----  -------------    ----                                    */
   /*          2    list directed     0                                      */
   /*          4    Iw                14                                     */
   /*          4    Fw.d              15                                     */
   /*          6    Dw.dEe            16                                     */
   /*          6    Ew.dEe            17                                     */
   /*          3    Gw.dEe            18                                     */
   /*        ext    B'sss'            19                                     */
   /*                                                                        */
   /*------------------------------------------------------------------------*/

   ndec = fiondec;
   scalf = fioscal;
   field = dspdig;
   freeform = 0;
   if(fiofspec == 0 || fioiwd == 0)
   {
      if(fioconv & ZDFLEXF) freeform = 1;
      if(ChrNsig <= 7)
      {
         fioiwd = 15;
         ndec = 7;
      }
      else
      {
         fioiwd = 24;
         ndec = 16;
      }
   }
   if(fiofspec)
   {
      switch(fiofspec)
      {
      case 14:
         ndec = 0;
         goto wval04;
      case 15: 
         if(scalf) value *= pow(10.0,(double)scalf);
         goto wval04;
      case 16:
         goto wval06;
      case 17:
         goto wval06;
      case 18:
         goto wval03;
      case 19:
         fionchar += FioPicture(value,fiobfmt,fiocrec+fionchar);
         return;
      default:
         fioier = EWVL_IFS;
         return;
      }
   }

   /*------------------------------------------------------------------------*/
   /*                                                                        */
   /* Step 2: List directed output is being done, then set the single        */
   /* precision convention to "G15.7E2" and the double precision convention  */
   /* to "G24.16D2". Then continue to step 3 to actually form the display.   */
   /*------------------------------------------------------------------------*/

   if(ChrNsig <= 7)
   {
      fioefw = 2;
      fioefc = 'E';
   }
   else
   {
      fioefw = 2;
      fioefc = 'D';
   }
   if((fionchar + fioiwd) >= fioffw) fiowtxt();

   /*------------------------------------------------------------------------*/
   /*                                                                        */
   /* Step 3: G format is being used. Form the basic display and thus obtain */
   /* the magnitude of the value. If that magnitude is in the range 0 to     */
   /* fiondec use F-format conventions; else E-format. Note that under       */
   /* F-format, blanks are stored at the back of the field where the         */
   /* E-format exponent information would be stored.                         */
   /*                                                                        */
   /*------------------------------------------------------------------------*/

wval03:
   ChrDoubleToChar(value,ndec,&nd,&minus,dspdig);
   field = dspdig;
   nc = strlen(field);
   if(nc > ndec && field[nc-1] == '0')
   {
      nc--;
      field[nc] = '\0';
   }
   if(nd >= 0 && nd < ndec) ChrShiftRight(field+nd,1,'.');
   else if(nd == ndec) *(field+nc) = '.';
   else goto wval06;
   nc++;
   if(fioefw == 0) fioefw = 2;
   filmem(field+nc,fioefw+2,' ');
   nc += (fioefw+2);
   goto wval05;

   /*------------------------------------------------------------------------*/
   /*                                                                        */
   /* Step 4: F format is being used. Form the basic display.                */
   /*                                                                        */
   /*------------------------------------------------------------------------*/

wval04:
   ChrDoubleToChar(value,ChrNsig,&nd,&minus,dspdig);
   field = dspdig;
   nc = nd + ndec;
   if(nc < 0)
   {
      nc = -nc;
      if(nc > 1) minus = 0;
      ChrShiftRight(field,nc,'0');
      nd += nc;
      nc = 0;
      *(field+ChrNsig) = '\0';
   }
   if(nc < ChrNsig)
   {
      if(ChrRoundValue(field,nc,ChrNsig))
      {
         if(nc != 0 || nd > 0) *(field+nc++) = '0';
         else *(field+nc++) = '1';
         nd++;
      }
   }
   else if(nc > ChrNsig) filmem(field+ChrNsig,nc-ChrNsig,'0');
   *(field+nc) = '\0';
   if(nd < 0)
   {
      nd = -nd;
      ChrShiftRight(field,nd,'0');
      nc += nd;
      nd = 0;
   }
   if(minus)
   {
      for(i = 0; i < nc; i++) if(*(field+i) != '0') break;
      if(i == nc) minus = 0;
   }
   if(fiondec >= 0)
   {
      ChrShiftRight(field+nd,1,'.');
      nc++;
   }
   else if(nc == 0 && nd == 0)
   {
      *field = '0';
      *(field+1) = '\0';
      nc = 1;
   }

   /*------------------------------------------------------------------------*/
   /*                                                                        */
   /* Step 5: Insert a leading zero, if needed, and then branch to step to   */
   /* complete the display.                                                  */
   /*                                                                        */
   /*------------------------------------------------------------------------*/

wval05:
   if(nc == 1 && fiondec >= 0)
   {
      ChrShiftRight(field,1,'0');
      nc++;
   }
   goto wval08;

   /*------------------------------------------------------------------------*/
   /*                                                                        */
   /* Step 6: E format is being used. Form the basic display.                */
   /*                                                                        */
   /*------------------------------------------------------------------------*/

wval06:
   if(value == 0.0) scalf = 0;
   if(scalf >= (ndec+2))
   {
      nc = fioiwd + 1;
      goto wval09;
   }
   if(scalf < 0) ndec += scalf;
   else if(scalf > 0) ndec++;
   if(ndec == 0)
   {
      nc = fioiwd + 1;
      goto wval09;
   }
   ChrDoubleToChar(value,ndec,&nd,&minus,dspdig);
   dspdig[ndec] = '\0';
   field = dspdig;
   nc = strlen(field);

   /*------------------------------------------------------------------------*/
   /*                                                                        */
   /* Step 7: Form the remainder of the E-format display.                    */
   /*                                                                        */
   /*------------------------------------------------------------------------*/

   if(scalf >= 0)
   {
      ChrShiftRight(field+scalf,1,'.');
      nc++;
   }
   else
   {
      ChrShiftRight(field,-scalf,'0');
      nc -= scalf;
      ChrShiftRight(field,1,'.');
      nc++;
   }
   nd -= scalf;
   *(field+nc++) = (char) fioefc;
   if(nd < 0)
   {
      nd = -nd;
      *(field+nc++) = '-';
   }
   else *(field+nc++) = '+';
   if(fioefw == 0) fioefw = 2;
   for(i = fioefw-1; i >= 0; i--)
   {
      *(field + nc + i) = ChrDigits[nd % 10];
      nd /= 10;
   }
   if(nd > 0)
   {
      nc = fioiwd + 1;
      goto wval09;
   }
   nc += fioefw;

   /*------------------------------------------------------------------------*/
   /*                                                                        */
   /* Step 8: Complete the display and end local processing.                 */
   /*                                                                        */
   /*------------------------------------------------------------------------*/

wval08:
   if((*field == '.') && (ChrFlags & CHR_LEADZERO))
   {
      nd = fioiwd;
      if(minus || (ChrFlags & CHR_PLUSSIGN)) nd--;
      if(nc < nd)
      {
         ChrShiftRight(field,1,'0');
         nc++;
      }
   }
   if(minus)
   {
      ChrShiftRight(field,1,'-');
      nc++;
   }
   else if(ChrFlags & CHR_PLUSSIGN)
   {
      ChrShiftRight(field,1,'+');
      nc++;
   }
wval09:
   if(freeform) while((nc > 0) && (field[nc-1] == ' ')) nc--;
   if(nc > fioiwd)
   {
      filmem(fiocrec+fionchar,fioiwd,'*');
      nc = fioiwd;
   }
   else
   {
      if(freeform) fioiwd = nc + 1;
      if(nc < fioiwd)
      {
         filmem(fiocrec+fionchar,fioiwd-nc,' ');
         fionchar += (fioiwd - nc);
      }
      cpymem(field,fiocrec+fionchar,nc);
   }
   fionchar += nc;
}

/*
;/doc/ ***********************************************************************
;
; ChrShiftRight: Shift character string right
;
; Synopsis of Service:
;
; #include "chrapi.h"                  Character processing interface
;
; void ChrShiftRight                   Shift character string right
; (
;    char* s,                          The string to be shifted
;    int   n,                          The number of positions to shift
;    int   fill                        The fill character to use
; )
;
; Description of Service:
;
; Shifts a character string right a specified number of places. The spaces
; thus created are set equal to the specified fill character. This service
; is typically used during detailed editing of displays during various
; numeric conversions.
;
; The most common error in using this service involves forgetting that the
; string must be null-terminated.
;
; Properties of Service:
;
; s          String to be shifted
;
; n          Number of places to be shifted
;
; fill       The fill character to be stored in the created places
;
; Return Value from Service: None
;
;/doc/ ************************************************************************
*/

#ifndef platform_h                /* Is this a standalone file */
#define STRGFUNC                  /* Gives access to the string services */
// #include "platform.h"             /* Define the platform hosting this code */
// #include "chrapi.h"               /* Character manipulation Api */
#endif /* platform_h */

#ifdef FPROTOTYPE
void ChrShiftRight(char* s, int n, int fill)
#else
void ChrShiftRight(s,n,fill)
char* s;                     /* String to be shifted */
int n;                       /* Number of places to shift */
int fill;                    /* The fill character */
#endif
{
   char* b;             /* Points to the back of the string */
   char* f;             /* Points to the front of the string */

    for(f = s+strlen(s),b = f+n; f >= s; f--,b--) *b = *f;
    for(f = s; f <= b; f++) *f = (char)(fill);
}

/*
;/doc/ ***********************************************************************
;
; fiofdata: Fortran I/O File Data
;
; Synopsis of Service:
;
; #include "pfclib.h"                  Pfc-Fortran runtime library interface
;
; void fiofdata                        Fortran I/O file data
; (
;    int   option;                     Specifies which data is being set
;    void* str;                        Information being used to set data
;    int   ns;                         String length or integer information
; )
;
; Description of Service:
;
; This service is used to specify the various file data options associated
; with the current FORTRAN file structure. The particular data being specified
; is defined by the "option" property as follows:
;
; Option  Description of data
; ------  -------------------
;   1     Specifies the file name to be assigned to the file. This name may
;         have no more than FIONMAX characters. If the name is NULL or if it
;         is all blank, then no name is assigned. If the file is opened with
;         no name assigned, then the open service itself will request a name
;         or will create a name depending upon the convention selected.
;
;   2     Specifies the status of the file. Only the first character of the
;         "strg" property is used as follows:
;         Char  Meaning
;         O,o   Old--the file already exists, do not create it.
;         N,n   New--the file does not exist, create it even if this means
;               destroying an existing file with the same name.
;         S,s   Scratch--the same as new, except that the file is removed
;               when it is closed.
;         U,u   Unknown--if the file exists it is opened, if it does not
;               exist then it is created.
;
;   3     Specifies the access type of the file. Only the first character
;         of the "strg" property is used as follows:
;         Char  Meaning
;         S,s   Sequential--the file is opened for sequential access.
;         D,d   Direct--the file is opened for direct access.
;         A,a   Append--the file is opened for sequential access, but is
;               positioned at the end-of-file initially
;
;   4     Specifies the form of the file. Only the first character of the
;         "strg" property is used as follows:
;         Char  Meaning
;         F,f   the records are formatted
;         U,u   the records are unformatted
;
;   5     Specifies the record size for the file as contained in the
;         property "ns". The property "strg" is ignored.
;
;   6     Specifies the convention for treating blanks in numeric input
;         fields. Only the first character of the "strg" property is used
;         as follows:
;         Char  Meaning
;         N     NULL--blank characters in numeric formatted input fields
;               are ignored, except that a field of all blanks is zero.
;         Z     ZERO--Blanks are treated as zeros.
;
;   7    Specifies that the file is readonly
;
;   8    Specifies that the file is to be open for shared accesss
;
;   9    Specifies the record type
;
;  10    Specifies the carriage control conventions
;
;  11    Specifies a pointer to the associated variable
;
;  12    Specifies the maximum number of record on the file
;
;  13    Specifies a pointer to receive the number of the unit assigned to
;        the file.
;
;  14    Specifies the associated buffer address along with its size.
;
;  15    Specifies a buffer block size
;
;  16    Specifies the file has some nonestandard use
;
; Properties of Service:
;
; option      Specifies which data is being specified
;
; str         String information
;
; ns          String length or integer information
;
; Return Value from Service: None
;
;/doc/ ************************************************************************
*/
#ifndef platform_h                /* Is this a standalone file */
#define MEMOFUNC                  /* Access to memory allocation functions */
#define PRIOFUNC                  /* Gives access to PROMULA I/O functions */
#define RAWMFUNC                  /* Gives access to raw memory functions */
#define STRGFUNC                  /* Gives access to the string functions */
// #include "platform.h"             /* Define the platform hosting this code */
// #include "pfclib.h"               /* Define PROMULA.FORTRAN library symbols */
#endif /* platform_h */

extern FIOFILE* fiocurf;          /* Pointer to current FORTRAN file */
extern int      fioconv;          /* Runtime convention flags */
extern FIOFILE  fioopnf[FIOMAX];  /* Describes all currently open files */

#ifdef FPROTOTYPE
void fiofdata(int option,void* str,int ns)
#else
void fiofdata(option,str,ns)
int option;                    /* Specifies which data is being specified */
void* str;                     /* String information */
int ns;                        /* String length or integer information */
#endif
{
   char* strg;
   // int nstr;
   int nc;

   strg = (char*) str;
   // nstr = ns;
   if(ns) switch(option)
   {
   case 1:                     /* Specify file name */
      while(ns && (*strg == ' '))
      {                           /* Strip any leading blanks */
         strg++;
         ns--;
      }
      while(ns && (*(strg+ns-1) == ' ')) ns--; /* Strip trailing blanks */
      if(ns >= FIONMAX) ns = FIONMAX - 1;
      if(fiocurf->fstat1 & FALREADYO)
      {
         nc = strlen(fiocurf->fname);
         if(nc == ns)
         {
            if(fifindex(strg,nc,fiocurf->fname,nc) != 0) break;
         }
         fioclose();
         *(txtfile*)&(fiocurf->ffile) = nultxtf;
         fiocurf->frsize = fiocurf->fmaxrc = 0; 
         fiocurf->fstat1 = 0;
         fiocurf->fstat = FUNKNOWN;
#ifdef BLANKNULL
         fiocurf->fstat |= FBLANK;
#endif
      }
      if(ns)
      {
         if(FioConvertName != NULL)
         {
            strg = FioConvertName(strg,ns);
            ns = strlen(strg);
         }
         cpymem(strg,fiocurf->fname,ns);
      }
      fiocurf->fname[ns] = '\0';
      break;
   case 2:                     /* Specify file status */
      if(fiocurf->fstat1 & FALREADYO) break;
      fiocurf->fstat |= FUNKNOWN;
      fiocurf->fstat ^= FUNKNOWN;
      if(*strg == 'S' || *strg == 's')
      {                           /* Scratch or Save */
         fiocurf->fstat |= FSCRATCH;
         if(*(strg+1) == 'A' || *(strg+1) == 'a')fiocurf->fstat ^= FSCRATCH;
      }
      else if(*strg == 'O' || *strg == 'o') fiocurf->fstat |= FEXIST;
      else if(*strg == 'U' || *strg == 'u') fiocurf->fstat |= FUNKNOWN;
      else if(*strg == 'K' || *strg == 'k') fiocurf->fstat |= FKEEP;
      else if(*strg == 'D' || *strg == 'd') fiocurf->fstat |= FDELETE;
      break;
   case 3:                     /* Specify file access type */
      if(*strg == 'D' || *strg == 'd')
      {
         fiocurf->fstat |= FDIRECT;
         fiocurf->fstat |= FBINARY;
      }
      else if(*strg == 'A' || *strg == 'a')
      {
         fiocurf->fstat |= FENDFIL;
         if(!(fiocurf->fstat & FEXIST)) fiocurf->fstat |= FUNKNOWN;
      }
      else if(*strg == 'T' || *strg == 't')
      {
         fiocurf->fstat1 |= FTERMINAL;
      }
      break;
   case 4:                     /* Spefify record form */
      if(*strg == 'U' || *strg == 'u') fiocurf->fstat |= FBINARY;
      else if(*strg == 'F' || *strg == 'f')
      {
         fiocurf->fstat |= FBINARY;
         fiocurf->fstat ^= FBINARY;
      }
      break;
   case 5:                     /* Specify record size */
      fiocurf->frsize = ns;
      break;
   case 6:                     /* Specify input blanks convention */
      fiocurf->fstat |= FBLANK;
      if(*strg != 'N' && *strg != 'n') fiocurf->fstat ^= FBLANK;
      break;
   case 7:                     /* Specifies that the file is readonly */
      fiocurf->fstat |= FUNKNOWN;
      fiocurf->fstat ^= FUNKNOWN;
      fiocurf->fstat |= FEXIST;
      fiocurf->fstat |= FREADONLY;
      break;
   case 8:                     /* Specifies that the file is shared */
      break;
   case 9:                     /* Specifies the record type */
      if(*strg == 'F' || *strg == 'f') fiocurf->fstat1 |= FFIXEDTY;
      break;
   case 10:                    /* Specifies carriage control conventions */
      fiocurf->fstat |= FCARRIG;
      if(fioconv & ZDFLEXF)
      {
         if(*strg != 'F' && *strg != 'f' && *strg != 'Y' && *strg != 'y')
            fiocurf->fstat ^= FCARRIG;
      }
      else
      {
         if(*strg != 'C' && *strg != 'c' && *strg != 'Y' && *strg != 'y')
            fiocurf->fstat ^= FCARRIG;
      }
      break;
   case 11:                    /* Specify pointer to associated variable */
      fiocurf->fassov = (LONG*)strg;
      fiocurf->fstat |= FDIRECT;
      fiocurf->fstat |= FBINARY;
      break;
   case 12:                    /* Specify max number of record on file */
      fiocurf->fmaxrc = ns;
      break;
   case 13:                    /* Specifies the assigned unit */
      *(LONG*)strg = fiocurf - fioopnf;
      break;
   case 14:
      if(!(fiocurf->fstat1 & FBLKSIZE))
      {
         fiocurf->fstat |= FINTERNAL;
         fiocurf->fassob = (char*) strg;
         fiocurf->fbufsiz = ns;
      }
      break;
   case 15:
#ifdef setbufsz
      if(!(fiocurf->fstat & FINTERNAL) && !(fiocurf->fstat1 & FBLKSIZE))
      {
         fiocurf->fstat1 |= FBLKSIZE;
         fiocurf->fassob = (char*)(getmem(ns));
         if(fiocurf->fassob != NULL)
         {
            fiocurf->fbufsiz = ns;
            fiocurf->fstat1 |= FBLKSIZE;
         }
      }
#endif
      break;
    case 16:                    /* buffer count (ignored) Flex PAD */
      if((fioconv & ZDFLEXF) == 0) fiocurf->fstat |= FBINARY;
      else
      {    /* This is POSITION="END" in Flex FORTRAN */
         if((*strg == 'E' || *strg == 'e') &&
            (*(strg+1) == 'N' || *(strg+1) == 'n') &&
            (*(strg+2) == 'D' || *(strg+2) == 'd')) fiocurf->fstat |= FENDFIL;
      }
      break;
    default:                    /* Ignore any other option settings */
      break;
   }
}

/*
;/hdr/ ************************************************************************
;
; Copyright 1986-1998 Promula Development Corporation ALL RIGHTS RESERVED
;
; name of module: FifStrg.c -- Fortran String Functions
;
; global symbols defined:
;
; useful notes and assumptions:
;
; initial author: (FKG) Fred Goodman, Promula Development Corporation
;         author: (LCC) Lois Carver, Promula Development Corporation
;
; edit 07/02/99 FKG: Unified the algorithm for allocating semipermanent string
;                    storage areas.
;
; edit 08/04/98 LCC: Converted headers and comments to new standard.
;
; edit 06/17/98 FKG: Changed names of global symbols to conform to the 
;                    Promula version 7.08 API naming conventions. 
;
; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
;                support for string manipulation.
;
;/hdr/ ************************************************************************
*/
#define MEMOFUNC                  /* Access to memory allocation function */
#define RAWMFUNC                  /* Gives access to raw memory functions */
#define STRGFUNC                  /* Gives access to string manipulation */
#define VARARGUS                  /* Gives access to variable arguments */
// #include "platform.h"             /* Define the platform hosting this code */
// #include "pfclib.h"               /* Define PROMULA.FORTRAN library symbols */
// #include "chrapi.h"               /* Define character manipulation interface */

#ifdef FPROTOTYPE
char*  FioStorage(int Length);
#else
extern char*  FioStorage();
#endif

extern int fioconv;               /* General dialect convention flags */

/*
;/doc/ ***********************************************************************
;
; name of routine: FIFCHAR -- FORTRAN Intrinsic Function CHAR
;
; Converts a numeric display code, or "lexical value" or "collalating
; weight" into its character code. The point of this function is that
; character values on the host processor are not necessarily the same as
; those on the machine for which a given FORTRAN program was written. All
; numeric display code references in a source FORTRAN program are passed
; through this function either by the translator directly or by the other
; runtime functions included in this library. Note that if you wish this
; function to return some value other than the host processor values then
; you most modify it. Typically this modification would take the form of a
; lookup table reference.
;
; calling parameters:
;
; return parameters:
;
; A pointer to a 1-character string containing the character corresponding
; to the display code.
;
; useful notes and assumptions:
;
; Element        Description of use
; -------        ------------------
; FORTRAN        Intrinsic function CHAR
;
; Move the display code into local string storage and return a pointer to the
; start of that strorage.
;
; initial author: (FKG) Fred Goodman, Promula Development Corporation
;
; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
;                support for string manipulation.
;
;/doc/ ************************************************************************
*/

#ifdef FPROTOTYPE
char* fifchar(int iv)
#else
char* fifchar(iv)
int iv;                        /* Value to be converted */
#endif
{
   char* c1;

   c1 = FioStorage(2);
   c1[0] = (char) iv;
   c1[1] = '\0';
   return c1;
}

/*
;/doc/ ***********************************************************************
;
; name of routine: FIFSTRGV -- FORTRAN String Value Conversion
;
; Copies a character string into a local buffer and then pads that buffer
; to 8 characters with blanks, so that the string can be compared to a
; character string which has been hidden in a noncharacter variable. Note
; that such hiding always pads the numeric with blanks out to its natural
; size. This function is needed for situations like
;
;     IF(K .EQ. 'Y')
;
; where K is a noncharacter variable. The translation of this would be
;
;     if(k == *(typeof(k)*)fifstrv("Y",1))
;
; calling parameters:
;
; return parameters:
;
; A pointer to a Buffer containing the string value.
;
; useful notes and assumptions:
;
; Element       Description of use
; -------       ------------------
; FORTRAN       Comparisons between numeric and string values
;
; Move the display code into a local string storage and return a pointer to the
; start of that strorage.
;
; initial author: (FKG) Fred Goodman, Promula Development Corporation
;
; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
;                support for string manipulation.
;
;/doc/ ************************************************************************
*/

#ifdef FPROTOTYPE
char* fifstrgv(char* str,int nc)
#else
char* fifstrgv(str,nc)
char* str;                     /* String to be converted */
int nc;                        /* Number of characters in string */
#endif
{
   int   i;
   char* c;

   c = FioStorage(10);
   if(nc > 8) nc = 8;
   for(i = 0; i < nc; i++) c[i] = *str++;
   while(i < 8) c[i++] = ' ';
   c[i] = '\0';
   return c;
}

/*
;/doc/ ***********************************************************************
;
; name of routine: FIFPUSHSTR -- FORTRAN Push A String
;
; Pushes a string into a temporary storage buffers so that dual
; calls to the same function can be compared.
;
; calling parameters:
;
; return parameters:
;
; A pointer to the result of the push.
;
; useful notes and assumptions:
;
; initial author: (FKG) Fred Goodman, Promula Development Corporation
;
; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
;                support for string manipulation.
;
;/doc/ ************************************************************************
*/

#ifdef FPROTOTYPE
char* fifpushstr(char* s1,int n1)
#else
char* fifpushstr(s1,n1)
char* s1;                      /* Pointer to string */
int n1;                        /* Length of string */
#endif
{
   char* ival;

   ival = FioStorage(n1);
   cpymem(s1,ival,n1);
   return ival;
}

/*
;/doc/ ***********************************************************************
;
; name of routine: FTNSUBS -- FORTRAN Substring Evaluation
;
; This function creates a string structure for a substring reference if a
; FORTRAN program. The operation it performs is minimal; however, it is
; needed in functional form to avoid evaluating the value of the substring
; bounds more than once. Note also that there is a dialectal variant here.
; Some allow the maximum string bound to exceed the specifiy string length
; while others do not.
;
; calling parameters:
;
; return parameters:
;
; None, the function is void; however, the parameter "subs" is updated to
; contain a pointer to the start of the substring and its length.
;
; useful notes and assumptions:
;
; Simply compute the start and length of the string.
;
; initial author: (FKG) Fred Goodman, Promula Development Corporation
;
; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
;                support for string manipulation.
;
;/doc/ ************************************************************************
*/

#ifdef FPROTOTYPE
void ftnssubs(string* subs,char* str,int slen,int ipos,int lpos)
#else
void ftnssubs(subs,str,slen,ipos,lpos)
string* subs;                  /* String spec to return substring data */
char* str;                     /* String which contains the substring */
int slen;                      /* Specified length of the string */
int ipos;                      /* Starting position of the substring */
int lpos;                      /* Ending position of the substring */
#endif
{
   if(fioconv & ZDSUBSS)
   {
      if(ipos > slen) ipos = slen;
      if(lpos > slen) lpos = slen;
   }
   ipos--;
   subs->a = str + ipos;
   subs->n = lpos - ipos;
}

/*
;/doc/ ***********************************************************************
;
; name of routine: FTNSCOPY -- FORTRAN String Copy
;
; This function copies a variable number of strings into a destination string.
; If at the end of that copy not all space in the destination is used, then
; it is filled with blanks.
;
; calling parameters:
;
; return parameters:
;
; None, the function is void.
;
; useful notes and assumptions:
;
; initial author: (FKG) Fred Goodman, Promula Development Corporation
;
; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
;                support for string manipulation.
;
;/doc/ ************************************************************************
*/

#ifdef VARUNIXV
void ftnscopy(va_alist) va_dcl
#else
void ftnscopy(char* s1, int ns1,...)
#endif
{
   va_list argptr;
   char* s;                  /* String being copied */
   int ns;                   /* Length of string being copied */
   char* sp[100];            /* Pointers to strings to be concatenated */
   int len[100];             /* Lengths of strings to be concatenated */
   int nstring;              /* Number of strings to be concatenated */
   int length;               /* Length of the result string */
   int i;
   char* res;

#ifdef VARUNIXV
   char* s1;
   int ns1;
   va_start(argptr);
   s1 = va_arg(argptr,char*);
   ns1 = va_arg(argptr,int);
#else
   va_start(argptr,ns1);
#endif
   nstring = length = 0;
   for(;;)
   {
      sp[nstring] = va_arg(argptr,char*);
      if(sp[nstring] == NULL) break;
      len[nstring] = va_arg(argptr,int);
      length += len[nstring];
      nstring++;
   }
   va_end(argptr);
   if(fioconv & (ZDVAXFT | ZDXCOPY))
   {
      if(length > ns1) length = ns1;
      res = s = (char *)getmem(length);
      if(s == NULL) s = s1;
   }
   else
   {
      s = s1;
      res = NULL;
   }
   for(i = 0; i < nstring && ns1 > 0; i++)
   {
      ns = len[i];
      if(ns > ns1) ns = ns1;
      ns1 -= ns;
      cpymem(sp[i],s,ns);
      s += ns;
   }
   if(res != NULL)
   {
      cpymem(res,s1,length);
      free(res);
   }
   if(ns1 > 0) filmem(s1+length,ns1,' ');
}

/*
;/doc/ ***********************************************************************
;
; name of routine: FTNCMS -- FORTRAN Compare Strings
;
; Compares two FORTRAN style strings to determine their lexical
; relationship. The comparison proceeds character by character until either
; the end of both strings is reached or until a specific character
; difference is found. If the strings are of unequal length, the shorter
; string is treated as though it were padded with blanks to the length of
; the longer string.
;
; Return value:
;
;  0  if no character difference is found
; -n  if a character in the first string is less than the corresponding
;     character in the second string.
; +n  if a character in the first string is greater that the corresponding
;     character in the secong string.
; 
; calling parameters:
;
; return parameters:
;
; useful notes and assumptions:
;
; If compare the equal length portions of the strings using the standard C
; string comparison function. If the fixed portions of the strings are
; equal, then check the tail of the remaining string to make certain that
; it is all blank. In FORTRAN to unequal length strings match if the tail
; of the longer string is all blank.
;
; initial author: (FKG) Fred Goodman, Promula Development Corporation
;
; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
;                support for string manipulation.
;
;/doc/ ************************************************************************
*/

#ifdef FPROTOTYPE
int ftncms(char* s1,int n1,char* s2,int n2)
#else
int ftncms(s1,n1,s2,n2)
char* s1;                      /* Pointer to first string */
int n1;                        /* Length of first string */
char* s2;                      /* Pointer to second string */
int n2;                        /* Length of second string */
#endif
{
   int dif;                  /* Difference value between the strings */
   int length;               /* Length of equal length portions */
   unsigned char blank;      /* Display code for a blank */
   int tail;                 /* Code indicating which string has tail */

   /*--------------------------------------------------------------------------
   ;
   ; Step 1: If either length is set at zero then the corresponding string
   ; is nul-terminated and its length can be detemined using the standard
   ; C strlen function.
   ;
   ;-------------------------------------------------------------------------*/

   if(n1 == 0 && s1 != NULL) n1 = strlen(s1);
   if(n2 == 0 && s2 != NULL) n2 = strlen(s2);
   if(n1 == 0 && n2 == 0) return 0;

   /*--------------------------------------------------------------------------
   ;
   ; Step 2: Compute the length of the shortest string, and if it is
   ; positive compare the front portions of the strings using the standard
   ; C string comparison function. If the front portion of the strings are
   ; unequal, or if they are of equal length, simply return the difference
   ; computed by the system function.
   ;
   ;-------------------------------------------------------------------------*/

   length = n1;
   dif = 0;
   if(length > n2)
   {
      length = n2;
      tail = 1;
   }
   else if(length == n2) tail = 0;
   else tail = 2;
   if(length > 0) dif = cmpstrn(s1,s2,length);
   if(dif != 0 || tail == 0) return dif;

   /*--------------------------------------------------------------------------
   ;
   ; Step 3: We are at this point only if we have strings of unequal length
   ; whose front portions are equal. We must now make certain that the
   ; remainder of the longer string is all blank. NOTE that the actual
   ; value of a blank varies dependending upon whether or not an extended
   ; internal character representation is being used.
   ;
   ;-------------------------------------------------------------------------*/

   blank = ' ';
   if(tail == 1)
   {
      s1 += length;
      n1 -= length;
      while(n1 > 0)
      {
         if((dif = *(unsigned char*) s1 - blank) != 0) return dif;
         n1 --;
         s1++;
      }
   }
   else
   {
      s2 += length;
      n2 -= length;
      while(n2 > 0)
      {
         if((dif = blank - *(unsigned char*) s2) != 0) return dif;
         n2 --;
         s2++;
      }
   }
   return 0;
}

/*
;/doc/ ***********************************************************************
;
; name of routine: FTNSCOMP -- FORTRAN String Comparison
;
; This function performs string comparisons in which at least one of the
; two operands represents a concatenation of smaller strings. Once any
; required concatenations have been performed, the comparison proceeds
; character by character until either the end of both strings is reached or
; until a specific character difference is found. If the strings are of
; unequal length, the shorter string is treated as though it were padded
; with blanks to the length of the longer string.
;
; calling parameters:
;
; return parameters:
;
;  0  if no character difference is found
; -n  if a character in the first string is less than the corresponding
;     character in the second string.
; +n  if a character in the first string is greater than the corresponding
;     character in the secong string.
;
; useful notes and assumptions:
;
; Compute the length and allocate storage for any concatenations that must
; be performed. Then perform the comparison using the standard Fortran
; string comparison function.
;
; initial author: (FKG) Fred Goodman, Promula Development Corporation
;
; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
;                support for string manipulation.
;
;/doc/ ************************************************************************
*/

#ifdef VARUNIXV
int ftnscomp(va_alist) va_dcl     /* FORTRAN String Comparison */
#else
int ftnscomp(char* s1, int ns1,...)
#endif
{
   va_list argptr;
   char* adr;                /* Dummy address */
   int i;                    /* Dummy counter */
   char* s[20];              /* Pointers to strings to be concatenated */
   int len[20];              /* Lengths of strings to be concatenated */
   int nstring;              /* Number of strings to be concatenated */
   int length;               /* Length of the result string */
   char* res1;               /* Pointer to the first result string */
   char* res2;               /* Pointer to the second result string */
   int len1;                 /* length of the first string */
   int alloc1;               /* Indicates if first string is allocated */
   int alloc2;               /* Indicates if second string is allocated */

#ifdef VARUNIXV
   char* s1;
   int ns1;
   va_start(argptr);
   s1 = va_arg(argptr,char*);
   ns1 = va_arg(argptr,int);
#else
   va_start(argptr,ns1);
#endif
   s[0] = s1;
   length = len[0] = ns1;
   nstring = 1;
   for(;;)
   {
      s[nstring] = va_arg(argptr,char*);
      if(s[nstring] == NULL) break;
      len[nstring] = va_arg(argptr,int);
      length += len[nstring];
      nstring++;
   }
   len1 = length;
   if(nstring == 1)
   {
      alloc1 = 0;
      res1 = s[0];
   }
   else
   {
      res1 = (char *)getmem(length);
      if(res1 == NULL)
      {
         puts("Unable to allocated scratch string storage.");
         exit(1);
      }
      alloc1 = 1;
      adr = res1;
      for(i = 0; i < nstring; i++)
      {
         cpymem(s[i],adr,len[i]);
         adr += len[i];
      }
   }
   length = nstring = 0;
   for(;;)
   {
      s[nstring] = va_arg(argptr,char*);
      if(s[nstring] == NULL) break;
      len[nstring] = va_arg(argptr,int);
      length += len[nstring];
      nstring++;
   }
   va_end(argptr);
   if(nstring == 1)
   {
      alloc2 = 0;
      res2 = s[0];
   }
   else
   {
      res2 = (char *)getmem(length);
      if(res2 == NULL)
      {
         puts("Unable to allocated scratch string storage.");
         exit(1);
      }
      alloc2 = 1;
      adr = res2;
      for(i = 0; i < nstring; i++)
      {
         cpymem(s[i],adr,len[i]);
         adr += len[i];
      }
   }
   i = ftncms(res1,len1,res2,length);
   if(alloc1) free(res1);
   if(alloc2) free(res2);
   return i;
}

/*
;/doc/ ***********************************************************************
;
; name of routine: FTNSAC -- FORTRAN Store a Character String
;
; Stores a FORTRAN style character string into another string and then pads
; that string with blanks. Note that some care must be taken with over-
; laying copies. Since not all platforms support an overlayed copy, this is
; done via an explicit loop.
;
; calling parameters:
;
; return parameters:
;
; None, the function is void.
;
; useful notes and assumptions:
;
; Make certain that no more than the length of the receiving string
; characters are sent. Then copy the string and pad the remainder of the
; receiving string with blanks.
;
; initial author: (FKG) Fred Goodman, Promula Development Corporation
;
; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
;                support for string manipulation.
;
;/doc/ ************************************************************************
*/

#ifdef FPROTOTYPE
void ftnsac(char* s1,int n1,char* s2,int n2)
#else
void ftnsac(s1,n1,s2,n2)
char* s1;                      /* Pointer to receiving string */
int n1;                        /* Length of receiving string */
char* s2;                      /* Pointer to sending string */
int n2;                        /* Length of sending string */
#endif
{
   int nmove;
   unsigned char blnk;

   if(n2 > n1) nmove = n1;
   else nmove = n2;
   if(s1 < s2 || s2 >= (s1 + nmove)) cpymem(s2,s1,nmove);
   else if(s1 > s2)
   {
      s1 += (nmove - 1);
      s2 += (nmove - 1);
      while(nmove-- > 0) *s1-- = *s2--;
      s1++;
   }
   if(n1 > n2)
   {
      blnk = ' ';
      filmem(s1+n2,n1-n2,blnk);
   }
}

/*
;/doc/ ***********************************************************************
;
; name of routine: FTNADS -- FORTRAN Add Strings
;
; Concatenates two FORTRAN style strings into a scratch storage array and
; returns a pointer to that array. Calls to this function may be nested.
;
; calling parameters:
;
; return parameters:
;
; A pointer to the result of the concatenation.
;
; useful notes and assumptions:
;
; initial author: (FKG) Fred Goodman, Promula Development Corporation
;
; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
;                support for string manipulation.
;
;/doc/ ************************************************************************
*/

#ifdef FPROTOTYPE
char* ftnads(char* s1,int n1,char* s2,int n2)
#else
char* ftnads(s1,n1,s2,n2)
char* s1;                      /* Pointer to first string */
int n1;                        /* Length of first string */
char* s2;                      /* Pointer to second string */
int n2;                        /* Length of second string */
#endif
{
   char* ival;

   /*--------------------------------------------------------------------------
   ;
   ; Step 1: If either length is set at zero then the corresponding string
   ; is nul-terminated and its length can be detemined using the standard
   ; C strlen function.
   ;
   ;-------------------------------------------------------------------------*/

   if(n1 == 0) n1 = strlen(s1);
   if(n2 == 0) n2 = strlen(s2);

   /*-------------------------------------------------------------------------
   ;
   ; Step 2: Get a temporary storage area large enouph to hold the two 
   ; strings along with a null byte. Note that FORTRAN does no require this
   ; byte, but its presence simplifies optimizing calls to this function
   ; via the trick above.
   ;
   ;-------------------------------------------------------------------------*/

   ival = FioStorage(n1+n2+1);

   /*--------------------------------------------------------------------------
   ;
   ; Step 3: Copy in the strings and return a pointer to the storage area.
   ;
   ;-------------------------------------------------------------------------*/

   cpymem(s1,ival,n1);
   cpymem(s2,ival+n1,n2);
   ival[n1+n2] = '\0';
   return ival;
}

/*
;/doc/ ***********************************************************************
;
; name of routine: FTNCAT -- FORTRAN Concatenate Strings
;
; Concatenates two FORTRAN C-style strings into a scratch storage array and
; returns a pointer to that array. Calls to this function may be nested.
;
; calling parameters:
;
; return parameters:
;
; A pointer to the result of the concatenation.
;
; useful notes and assumptions:
;
; initial author: (FKG) Fred Goodman, Promula Development Corporation
;
; creation date: 03/14/86 as part of Promula.Fortran Version 1.00 to provide
;                support for string manipulation.
;
;/doc/ ************************************************************************
*/

#ifdef FPROTOTYPE
char* ftncat(char* s1,char* s2)
#else
char* ftncat(s1,s2)
char* s1;                      /* Pointer to first string */
char* s2;                      /* Pointer to second string */
#endif
{
   char* ival;
   int n1;                 /* Length of first string */
   int n2;                 /* Length of second string */

   /*--------------------------------------------------------------------------
   ;
   ; Step 1: Determine the lengths of the strings.
   ;
   ;-------------------------------------------------------------------------*/

   n1 = strlen(s1);
   n2 = strlen(s2);

   /*-------------------------------------------------------------------------
   ;
   ; Step 2: Get a temporary storage area large enouph to hold the two 
   ; strings along with a null byte.
   ;
   ;-------------------------------------------------------------------------*/

   ival = FioStorage(n1+n2+1);

   /*--------------------------------------------------------------------------
   ;
   ; Step 3: Copy in the strings and return a pointer to the storage area.
   ;
   ;-------------------------------------------------------------------------*/

   cpymem(s1,ival,n1);
   cpymem(s2,ival+n1,n2);
   ival[n1+n2] = '\0';
   return ival;
}

/*
;/doc/ ***********************************************************************
;
; name of routine: fifitoc
;
; This function converts an integer value into a string and returns a 
; semi-permanent pointer to its start.
;
; calling parameters:
;
; int   Value    The integer value to be converted
;
; return parameters:
;
; A pointer to the storage area containing the string representation.
;
; useful notes and assumptions:
;
; initial author: (FKG) Fred Goodman, Promula Development Corporation
;
; creation date: 07/02/99
;
;/doc/ ************************************************************************
*/
#ifdef FPROTOTYPE
char* fifitoc(int Value)
#else
char* fifitoc(Value)
int Value;
#endif
{
   char s[8];
   char* Storage;
   int nc;

   ChrShortString(Value,s,0);
   nc = strlen(s);
   Storage = FioStorage(nc+1);
   cpymem(s,Storage,nc+1);
   return Storage;
}

/*
;/hdr/ ************************************************************************
;
; FifBlkd.c: Fortran Block Data
;
;/hdr/ ************************************************************************
*/
// #include "platform.h"             /* Define the platform hosting this code */
// #include "pfclib.h"               /* Define PROMULA.FORTRAN library symbols */

/*
;/doc/ ***********************************************************************
;
; fifblkb: Dummy FORTRAN Block Data
;
; Synopsis of Service:
;
; #include "fortran.h"                   pFortran runtime library interface
;
; void ftnblkd()                         Dummy FORTRAN block data
;
; Description of Service:
;
; This service represents a dummy entry point for those FORTRAN programs
; which have no BLOCK DATA subprograms. It is called by the FORTRAN runtime
; initialization function.
;
; Properties of Service: None
;
; Return Value from Service: None
;
;/doc/ ************************************************************************
*/
#ifdef FPROTOTYPE
void ftnblkd(void)
#else
void ftnblkd()
#endif
{
   return;
}

/*
;/doc/ ***********************************************************************
;
; fiogetname: Get Name of File from OS
;
; Synopsis of Service:
;
; #include "pfclib.h"                  Pfc-Fortran runtime library interface
;
; int fiogetname                       Get name of file from OS
; (
;    char* lun                         Contains logical unit number of file
;    char* filename                    Returns name of file, if found
; )
;
; Description of Service:
;
; When there is no name associated with a file by the time that it is opened
; the runtime first uses this service to see if a name can be supplied by the
; operating system. This service is normally empty and simply returns a failure
; indication.
;
; Properties of Service:
;
; lun         This string contains the logical unit number of the file to be
;             opened. By this point in time this is the only identifying
;             information about the file.
;
; filename    If a name can be determined for the file based on its unit number
;             then that name is stored here. If no name is located then this
;             property is not disturbed.
;
; Return Value from Service:
;
; If a name can be determined, then the length of the name is returned; else
; a zero is returned.
;
;/doc/ ************************************************************************
*/
#ifndef platform_h                /* Is this a standalone file */
#define MEMOFUNC                  /* Access to memory allocation functions */
#define PRIOFUNC                  /* Gives access to PROMULA I/O functions */
#define RAWMFUNC                  /* Gives access to raw memory functions */
#define STRGFUNC                  /* Gives access to the string functions */
// #include "platform.h"             /* Define the platform hosting this code */
// #include "pfclib.h"               /* Define PROMULA.FORTRAN library symbols */
#endif /* platform_h */

#ifdef FPROTOTYPE
int fiogetname(char* /* lun */,char* /* filename */)
#else
int fiogetname(lun,filename)
char* lun;                     /* Contains the logical unit number */
char* filename;                /* Returns name of file if found */
#endif
{
   // lun;
   // filename;
   return 0;
}

/*
;/hdr/ ************************************************************************
;
; chrreal.c: Real Character Representations
;
; This provider converts real(float and double) values from and to their
; character representations.
;
;/hdr/ ************************************************************************
*/
#define CHARTYPE                  /* Gives access to character type info */
#define STRGFUNC                  /* Gives access to the string services */
// #include "platform.h"             /* Define the platform hosting this code */
// #include "chrapi.h"               /* Character manipulation Api */
#ifdef DGEPLAT
#define DIGPREC     18       /* max # of significant digits */
#endif
#ifdef UNXPLAT
#define DIGPREC     18       /* max # of significant digits */
#endif /* UNXPLAT */
#ifdef AIXPLAT
#define HIGHEXPO             /* has high precision exponent */
#define DIGPREC     18       /* max # of significant digits */
#endif /* AIXPLAT */
#ifdef FUJPLAT
#define HIGHEXPO             /* has high precision exponent */
#define DIGPREC     18       /* max # of significant digits */
#endif /* FUJPLAT */
#ifdef A86PLAT
#define HIGHEXPO             /* has high precision exponent */
#define DIGPREC     18       /* max # of significant digits */
#endif /* A86PLAT */
#ifdef AN4PLAT
#define HIGHEXPO             /* has high precision exponent */
#define DIGPREC     18       /* max # of significant digits */
#endif /* AN4PLAT */
#ifdef UCSPLAT
#define HIGHEXPO             /* has high precision exponent */
#define DIGPREC     18       /* max # of significant digits */
#endif /* UCSPLAT */
#ifdef AZTPLAT
#define DIGPREC     17       /* max # of significant digits */
#endif /* AZTPLAT */
#ifdef MPWPLAT
#define DIGPREC     17       /* max # of significant digits */
#endif /* MPWPLAT */
#ifdef TCCPLAT
#include <string.h>          /* declarations for "memset+memcpy" */
#include <stdlib.h>          /* declaration for "ecvt" */
#define HASECVTF             /* platform has ecvt */
#endif /* TCCPLAT */
#ifdef THCPLAT
#define DIGPREC     17       /* max # of significant digits */
#endif /* THCPLAT */
#ifdef TSOPLAT
#define DIGPREC     17       /* max # of significant digits */
#endif /* TSOPLAT */
#ifdef VMSPLAT
#include <string.h>          /* declarations for "memset+memcpy" */
#include <stdlib.h>          /* declaration for "ecvt" */
#define HASECVTF             /* platform has ecvt */
#endif /* VMSPLAT */
#ifdef SKYPLAT
#define HIGHEXPO              /* has high precision exponent */
#define DIGPREC     18        /* max # of significant digits */
#endif /* SKYPLAT */
#ifdef LCCPLAT
#include <string.h>           /* declarations for "memset+memcpy" */
#include <math.h>             /* declaration for "ecvt" */
#define HASECVTF              /* platform has ecvt */
#endif /* LCCPLAT */

/*
;/doc/ ***********************************************************************
;
; ChrCharToDouble: Convert characters to double
;
; Synopsis of Service:
;
; #include "chrapi.h"                  Character processing interface
;
; double ChrCharToDouble               Convert characters to double
; (
;    char* str,                        String containing the double 
;    int   nstr                        Number of characters to be examined
; )
;
; Description of Service:
;
; This service converts an alphanumeric string containing a number in
; scientific notation to a double precision floating point number. The string
; can contain optional leading blanks, an integer part, a fractional part, and
; an exponent part. The integer part consists of an optional sign followed by
; zero or more decimal digits. The fractional part is a decimal point followed
; by zero or more decimal digits. The exponent part consists of an 'E', 'e',
; 'D', 'd', 'Q', or 'q' followed by an optional sign and a sequence of decimal
; digits. 
;
; Properties of Service:
;
; str     Contains the alphanumeric string to be converted.
;
; nstr    Contains the number of characters in the string. Note that the string
;         need not be null-terminated.
;
; Return Value from Service:
;
; The service itself returns the double precision value of the string. If the
; conversion encounters a character which is not part of the notation then the
; global variable 'ChrIhist' returns the position of that character. The global
; variable 'ChrNdec" returns the number of decimal places in the fractional part
; of the number plus 1. Thus a value of zero means there was no decimal point
; and one means there was a decimal, but no fractional digits.
;
;/doc/ ************************************************************************
*/

#ifdef FPROTOTYPE
double ChrCharToDouble(char *str,int nstr)
#else
double ChrCharToDouble(str,nstr)
char* str;
int nstr;
#endif
{    
double dval;
int vexp;
char* sadr;
char* lstr;
int sign;
int ndig;
int idig;
int nexp;

   /*-------------------------------------------------------------------------
   ;
   ; Step 1: Initialize the needed control variables. Note that the string is 
   ; temporarily made to be nul-terminated to simplify the logic. Note also
   ; that if the string string is empty then a zero is simply returned.
   ;
   ;-------------------------------------------------------------------------*/

    ChrIsdble = ChrIhist = ChrError = ChrNdec = 0;
    dval = 0.0;
    while(*str == ' ' && nstr > 0) {
        str++;
        nstr--;
    }
    if(!nstr) goto ReturnDoubleValue;
    sign = vexp = ndig = nexp = 0;
    sadr = str;
    idig = *(str + nstr);
    *(str + nstr) = '\0';

    /*-------------------------------------------------------------------------
    ;
    ; Step 2: If there is a leading sign, set the sign indicator accordingly.
    ;
    ;------------------------------------------------------------------------*/

    if(*str == '-' || *str == '+')  if(*str++ == '-') sign++;

    CalculateValue: /*---------------------------------------------------------
    ;
    ; Step 3: Calculate the value of the string and the number of decimal
    ; places.
    ;
    ;------------------------------------------------------------------------*/

    while (ChrTypeFlags[(int)*str] & CNUMBER)
    {
        ndig++;
        dval *= 10.0;
        dval += (int)(ChrInformation[(int)*str]);
        str++;
        if(ChrNdec) ChrNdec++;
    }
    if(*str == '.' && !ChrNdec) {
        str++;
        ChrNdec++;
        goto CalculateValue;
    }
    if(ChrNdec) vexp = 1-ChrNdec;

    /*-------------------------------------------------------------------------
    ;
    ; Step 4: If there is an exponent specified, calculate its value.
    ;
    ;------------------------------------------------------------------------*/

    if(*str == 'e' || *str == 'E' || *str == 'd' || *str == 'D' ||
       *str == 'Q' || *str == 'q') {
        if(*str != 'e' && *str != 'E') ChrIsdble = 1;
        lstr = str;
        str++;
        if(*str == '-' || *str == '+' || (ChrTypeFlags[(int)*str] & CNUMBER)) {
            if(!ChrNdec) ChrNdec = 1;
            nexp = ChrShortFromString(&str,0);
            if(nexp < 0 && vexp < 0) dval *= ChrPowerOfTen(nexp);
            else vexp += nexp;
        }
        else str = lstr;
    }

    /*------------------------------------------------------------------------
    ;
    ; Step 5: Adjust the value by the appropriate power of ten and by any sign.
    ;
    ;------------------------------------------------------------------------*/

    if(vexp) dval *= ChrPowerOfTen(vexp);
    if(sign) dval = -dval;

    /*------------------------------------------------------------------------
    ;
    ; Step 6: If the end of string has not been reached, set the position of
    ; the ending character and restore the trailing digit.
    ;
    ;------------------------------------------------------------------------*/

    if(*str)
    {
       ChrError = CHRERR_BADDIGITS;
       ChrIhist = str - sadr + 1;
    }
    *(sadr + nstr) = (char)(idig);

    ReturnDoubleValue: /*------------------------------------------------------
    ;
    ; Step 7: Return the current value to the calling routine.
    ;
    ;------------------------------------------------------------------------*/

    return dval;
}

/*
;/doc/ ***********************************************************************
;
; ChrPowerOfTen: Compute a power of 10 multiplier
;
; Synopsis of Service:
;
; #include "chrapi.h"                  Character processing interface
;
; double ChrPowerOfTen                 Compute a power of 10 multiplier
; (
;   int expon                          The exponent of the desired power
; )
;
; Description of Service:
;
; This service computes the value of 10.0 raised to a given integer power.
; It returns the double precision power of ten as computed.
;
; Internally, the service has a table of ten raised to the various powers of
; two. The service merely combines these powers, using the fact that any
; number can be expressed as the sum of powers of 2.
;
; Properties of Service:
;
; expon    The exponent for the desired power
;
; Return Value from Service:
;
; The double precision value of ten to the desired power.
;
;/doc/ ************************************************************************
*/
#ifdef FPROTOTYPE
double ChrPowerOfTen(int expon)
#else
double ChrPowerOfTen(expon)
int expon;
#endif
{
   static double p10[6] =
   {
      1.0e+32,1.0e+16,1.0e+8,1.0e+4,1.0e+2,1.0e+1
   };
   int sign;
   double ans;
   int i;
   int p2;

   /*--------------------------------------------------------------------------
   ;
   ; Step 1: If the exponent is zero, simply return 10 to the 0 which is
   ; 1.0.
   ;
   ;-------------------------------------------------------------------------*/

   if(expon == 0) return 1.0;

   /*--------------------------------------------------------------------------
   ;
   ; Step 2: If the exponent is negative, set a sign indicator and convert
   ; it to positive form. Then if the absolute value of the exponent is too
   ; large, simply return a zero.
   ;
   ;-------------------------------------------------------------------------*/

   if(expon < 0)
   {
      sign = 1;
      expon = -expon;
   }
   else sign = 0;
   if(expon >= 9999) return 0.0;

   /*--------------------------------------------------------------------------
   ;
   ; Step 3: Move through the table of binary powers of 10 and compute the
   ; value of the result via successive multiplications.
   ;
   ;-------------------------------------------------------------------------*/

   ans = 10.0;
   i = 0;
   p2 = 32;
   for(i = 0, p2 = 32; i < 6 && expon > 0; i++, p2 /= 2)
   {
      while(expon > p2)
      {
         ans *= p10[i];
         expon -= p2;
      }
   }

   /*--------------------------------------------------------------------------
   ;
   ; Step 4: If the exponent was negative, take the reciprocal of the value
   ; Then return the value to the calling service.
   ;
   ;-------------------------------------------------------------------------*/

   if(sign) ans = 1.0 / ans;
   return ans;
}

/*
;/doc/ ***********************************************************************
;
; ChrDoubleToChar: Convert Double to Raw Character Form
;
; Synopsis of Service:
;
; #include "chrapi.h"                  Character manipulation services
;
; void ChrDoubleToChar                 Convert double to raw display form
; (
;    double val                        The double to be converted
;    int    ndig                       The number of digits to be created
;    int*   pdecpt                     Returns position of decimal point
;    int*   psign                      Returns sign of value
;    char*  dspdig                     Returns raw string form
; )
;
; Description of Service:
;
; This service converts a double precision floating point value into a raw
; character form. ANSI C expects that all conversions of floating point values
; to string be performed via the "sprintf" service. Though this can be done,
; most generalized applications, prefer to perform their own editing 
; operations, and require only a raw conversion be performed.
;
; This service has a different implementation depending upon the platform
; hosting it. Many platforms have an "ecvt" C-service whose operation is 
; very similar to this one. For those platforms that service is called.
;
; For other platforms we must decompose the floating point value ourselves,
; being careful to assume the correct possible range in the exponent which
; is also platform dependent. There are three constants that must be defined
; for each platform:
;
; Constant  Description
; --------  -----------
; DIGPREC   The maximum number of significant digits in a double precision
;           value
; HIGHEXPO  If defined then the double precision representation has a maximum
;           magnitude of E+256 else a miximum magnitude of E+64.
; HASECVTF  If defined then the platform has an "ecvt" service for doing raw
;           double precision to character conversion.
;
; Properties of Service:
;
; double val      The value to be converted to character form
;
; int    ndig     The number of digits to produce. Precisely this many digits
;                 are produced.
;
; int*   pdecpt    Returns position of decimal point with respect to the
;                  beginning of the string.
;
; int*   psign     Retuns zero if the value was non-negative; else it returns
;                  one if the value was negative.
;
; char*  dspdig    Returns the string produced. It contains precisely "ndig"
;                  digits followed by a nul-byte. If the number of digits in
;                  "val" exceeds "ndig" then the last digit is rounded; if the
;                  number of digits is less that "ndig" then it is padded with
;                  zeros.
;
; Return Value from Service: None
;
;/doc/ ************************************************************************
*/
#ifdef FPROTOTYPE
void ChrDoubleToChar(double val,int ndig,int* pdecpt,int* psign, char* dspdig)
#else
void ChrDoubleToChar(val,ndig,pdecpt,psign,dspdig)
double val;                       /* Value to be converted */
int ndig;                         /* Number of digits to produce */
int* pdecpt;                      /* Returns position of decimal point */
int* psign;                       /* Returns the sign of the value */
char* dspdig;                     /* Returns the string produced */
#endif
{

/*-----------------------------------------------------------------------------
;
; Section I: Using the information for platform.h compile this code if an
; "ecvt" service is available.
;
;---------------------------------------------------------------------------*/
#ifdef HASECVTF
   char* s1;

   if(val != 0.0) 
   {
      s1 = ecvt(val,ndig,pdecpt,psign);
      memcpy(dspdig,s1,strlen(s1)+1);
   }
   else 
   {
      memset(dspdig,'0',ndig);
      dspdig[ndig] = '\0';
      *psign = *pdecpt = 0;
   }
#else
/*----------------------------------------------------------------------------
;
; Section II: Using the information in platform.h compile this code if no 
; "ecvt" service is available. In this case we must unroll the digits
; ourselves. Note that platforms differ as to the width of the exponent in
; their double precision binary format.
;
;----------------------------------------------------------------------------*/
#ifdef HIGHEXPO
#define MAXEXPON    256           /* Maximum exponent */
   static double negtab[10] = 
   {
      1e-256,1e-128,1e-64,1e-32,1e-16,1e-8,1e-4,1e-2,1e-1,1.0
   };
   static double postab[9] = 
   {
      1e+256,1e+128,1e+64,1e+32,1e+16,1e+8,1e+4,1e+2,1e+1
   };
#else
#define MAXEXPON    64             /* Maximum exponent */
   static double negtab[8] = 
   {
      1.0e-64,1.0e-32,1.0e-16,1.0e-8,1.0e-4,1.0e-2,1.0e-1,1.0
   };
   static double postab[7] = 
   {
      1.0e64,1.0e32,1.0e16,1.0e8,1.0e4,1.0e2,1.0e1
   };
#endif /* HIGHEXPO */
   int decpt;
   int n;
   int powr;
   int i;
   char *p;

   *psign = (val < 0) ? ((val = -val),1) : 0;
   ndig = (ndig < 0) ? 0 : (ndig < 30) ? ndig : 30;
   if(val < negtab[0] || val > postab[0] ) 
   {
      for (p = dspdig; p < dspdig+ndig; p++) *p = 0;
      decpt = 0;
   }
   else 
   {                              /* Adjust so that 1 <= val < 10 */
      decpt = 1;
      powr = MAXEXPON;
      i = 0;
      while (val < 1) 
      {
         while (val < negtab[i + 1]) 
         {    
            val /= negtab[i];
            decpt -= powr;
         } 
         powr >>= 1;
         i++;
      }
      powr = MAXEXPON;
      i = 0;
      while (val >= 10) 
      {
         while (val >= postab[i]) 
         {    
            val /= postab[i];
            decpt += powr;
         }
         powr >>= 1;
         i++;
      }

      /*-----------------------------------------------------------------------
      ;
      ; Pick off digits 1 by 1 and stuff into dspdig. Do 1 extra digit for
      ; rounding purposes.
      ;
      ;----------------------------------------------------------------------*/

      for(p = dspdig; p <= dspdig+ndig; p++) 
      {
         if (p >= dspdig+DIGPREC) *p = 0;
         else 
         {    
            n = (int)(val);
            *p = (char)(n);
            val = (val - (double) n) * 10.0;
         }
      }
      if (*--p >= 5) 
      {
         for(;;) 
         {
            --p;
            if (*p != 9) 
            {    
               (*p)++;
               break;
            }
            *p = 0;
            if (p == dspdig) 
            {
               ndig += 1;
               decpt++;
               dspdig[0]++;
               break;
            }
         } 
      } 
   }
   *pdecpt = decpt;
   for(i = 0; i < ndig; i++) dspdig[i] = ChrDigits[(int)dspdig[i]];
   dspdig[ndig] = '\0';
#endif /* HASECVTF */
}

/*
;/doc/ ***********************************************************************
;
; ChrRoundValue: Round display of value
;
; Synopsis of Service:
;
; #include "chrapi.h"                  Character processing interface
;
; int ChrRoundValue                    Round display of value
; (
;    char* dspdig,                     The value display
;    int   ndigit,                     The number of output digits desired
;    int   length                      The length of the display
; )
;
; Description of Service:
;
; Truncates and rounds a numeric string of digits. It modifies the content of
; the dspdig argument and returns the carry value from the round. If the input
; string consists of a sequence of "999.." such that all become rounded to
; zero, then the output string will contain "100..." and the service will
; return a value of 1; else it will return a value of 0.
;
; Properties of Service:
;
; dspdig    Contains the string of numeric digits to be rounded. It may
;           contain only numeric digits.
;
; ndigit    Specifies the number of digits desired in the rounded result.
;
; length    Specifies the total number of digits in the string as input.
;
; Return Value from Service:
;
; A one or a zero as described above.
;
;/doc/ ************************************************************************
*/
#ifdef FPROTOTYPE
int ChrRoundValue(char* dspdig,int ndigit,int length)
#else
int ChrRoundValue(dspdig,ndigit,length)
char* dspdig;                     /* Digit string to be rounded */
int ndigit;                       /* Number of rounded digits */ 
int length;                       /* Length of digit string */
#endif
{
   int idig;                 /* The return value */

   /*--------------------------------------------------------------------------
   ;
   ; Step 1: Initialize the return value at zero, then if the (ndigit+1)th
   ; digit is less than 5, simply truncate the string.
   ;
   ;-------------------------------------------------------------------------*/

   idig = 0;
   if(ChrInformation[(int)dspdig[ndigit]] < 5) goto TruncateString;

   /*--------------------------------------------------------------------------
   ;
   ; Step 2: If the rounding digit is the last digit and is exactly equal
   ; to 5, then simply truncate the string.
   ;
   ;-------------------------------------------------------------------------*/

   if(dspdig[ndigit] == '5' && (ndigit+1) == length) goto TruncateString;

   /*--------------------------------------------------------------------------
   ;
   ; Step 3: Round the last digit by changing all trailing nines to zero
   ; and adding one to the first digit less than nine. If there is no such
   ; digit, make the first digit a zero and set the return value to 1.
   ;
   ;-------------------------------------------------------------------------*/

   for(idig = ndigit-1; idig >= 0 && dspdig[idig] == '9'; idig--)
      dspdig[idig] = '0'; 
   if(idig >= 0)
   {
      dspdig[idig]++;
      idig = 0;
   }
   else
   {
      dspdig[0] = '1';
      idig = 1;
   }

   TruncateString: /*---------------------------------------------------------
   ;
   ; Step 4: Truncate the string and end local processing.
   ;
   ;-------------------------------------------------------------------------*/

   dspdig[ndigit] = '\0';
   return idig;
}
/*
;/doc/ ************************************************************************
;
; ChrDoubleToString: Convert double value to string
;
; Synopsis of Service:
;
; #include "chrapi.h"                  Character processing interface
;
; int ChrDoubleToString                Convert double value to string
; (
;    char*  dspdig                     Returns the converted string
;    double val                        The value to be converted
;    int    nsigdig                    Number of significant digits
;    int    cnvflags                   Conversion control flags
;    int    width                      Width of display field
;    int    ndec                       Number of decimal places
; )
;
; Description of Service:
;
; This service converts a floating point number to a character string. The
; conversion process and format is controlled by a set of global variables
; and decision flags. The logical decisions that have to be considered are
; as follows:
;
; Decision       General description
; --------       -------------------
; CHR_EFORMAT    Use E-format to display the value regardless of any other
;                conditions.
; CHR_ERNGCHK    Use E-format if the value is outside of some specified range:
;                10**eminval greater than val or 10**emaxval less than val.
; CHR_GFORMAT    Use E-format only if the field provided is not large to
;                accomodate an F-format display of the value.
; CHR_DOLRSIGN   Precede the display with a dollar sign.
; CHR_PLUSSIGN   Should the display always by preceded by a plus sign
; CHR_TRAILZERO  Should trailing decimal zeroes be deleted
; CHR_TRAILDECP  If the value has no decimal digits should trailing decimal
;                point be displayed.
; CHR_TRAILBLNK  Should the display always be followed by a trailing blank.
; CHR_COMMAFLG   Should commas be inserted in the number
; CHR_LEADZERO   Should leading zeros be displayed.
; CHR_EFMTPLUS   Should plus sign be included in E-format display
; CHR_MINUSPREN  Should minus sign be shown as prentheses
; CHR_ASTERIX    Should overflow fields be filled with asterixs
;
; Properties of Service:
;
; dspdig    Returns the converted string with a null-termination character.
;
; val       Contains the double-precision value to be converted.
;
; nsigdig   Number of significant digits
;
; cnvflags  Conversion control flags
;
; width     Width of display field
;
; ndec      Number of decimal places
;
; Return Value from Service:
;
; This service returns the number of characters formed by the conversion, not
; counting the terminating null-character.
;
;/doc/ ************************************************************************
*/
#ifdef FPROTOTYPE
int ChrDoubleToString(char* dspdig,double val,int nsigdig,int cnvflags,
                      int width,int ndec)
#else
int ChrDoubleToString(dspdig,val,nsigdig,cnvflags,width,ndec)
   char*  dspdig;                  /* Returns the converted string */
   double val;                     /* The value to be converted */
   int    nsigdig;                 /* Number of significant digits */
   int    cnvflags;                /* Conversion control flags */
   int    width;                   /* Width of display field */
   int    ndec;                    /* Number of decimal places */
#endif
{
   char* back;
   int   dpoints;
   int   fieldw;
   int   itemp;
   int   ewth;
   int   sign;
   char  String[32];

   /*----------------------------------------------------------------------------
   ;
   ; Step 1: Convert the value into a raw digits only display form containing
   ; no more than the specified number of significant digits and copy it into
   ; local storage.
   ;
   ;------------------------------------------------------------------------*/

   if(val != 0.0) ChrDoubleToChar(val,nsigdig,&dpoints,&sign,dspdig);
   else {
       for(itemp=0; itemp < nsigdig; itemp++) dspdig[itemp] = '0';
       dspdig[nsigdig] = '\0';
       sign = dpoints = 0;
   }
   /*--------------------------------------------------------------------------
   ;
   ; Step 2: Calculate a first estimate of the maximum digits available for
   ; the display based on the externally supplied field width and the dollar
   ; sign and plus sign flags.
   ;
   ;-------------------------------------------------------------------------*/

   if(width) fieldw = width;
   else fieldw = 29;
   if(fieldw < 0) fieldw = -fieldw;
   if(fieldw > 29) fieldw = 29;
   if(cnvflags & CHR_DOLRSIGN) fieldw--;
   if(sign) if(cnvflags & CHR_MINUSPREN) fieldw -= 2; else fieldw--;
   else if(cnvflags & CHR_PLUSSIGN) fieldw--;
   if(cnvflags & CHR_TRAILBLNK) fieldw--;

   /*--------------------------------------------------------------------------
   ;
   ; Step 3: Do the non G-format checks for E-format and branch to step 5 if
   ; E-format is to be used.
   ;
   ;-------------------------------------------------------------------------*/

   if(cnvflags & CHR_EFORMAT) goto ftoa050;
   if(val == 0.0) goto ftoa070;
   if(cnvflags & CHR_ERNGCHK)
       if((dpoints < ChrEminfmt || dpoints > ChrEmaxfmt)) goto ftoa050;

   /*--------------------------------------------------------------------------
   ;
   ; Step 4: If G-format is to be used do the additional checks to determine if
   ; the value can be displayed using F-format. If so, branch to step 7.
   ;
   ;-------------------------------------------------------------------------*/

   if(!(cnvflags & CHR_GFORMAT)) goto ftoa070;
   if(dpoints > 0) {
       itemp = fieldw;
       if(ndec) itemp -= (ndec + 1);
       if(itemp < 0)goto ftoa050;
       itemp -= dpoints;
       if(itemp < 0)goto ftoa050;
       if((cnvflags & CHR_COMMAFLG) && (dpoints > 3)) itemp -= ((dpoints-1)/3);
       if(itemp >= 0) goto ftoa070;
   }
   else if((ndec + dpoints) > 0) goto ftoa070;
   if(!(cnvflags & CHR_PLUSSIGN) && (width > 0) && !(cnvflags & CHR_PADBLANK))
   {
        fieldw--;
        cnvflags |= CHR_PLUSSIGN;
   }

   ftoa050: /*----------------------------------------------------------------
   ;
   ; Step 5: E-format is to be used. Calculate the number of digits which can
   ; be shown and round down the display if necessary.
   ;
   ;-------------------------------------------------------------------------*/

   if(val == 0.0) dpoints = ChrEmantisa;
   if(ChrEfmtwth) fieldw -= ChrEfmtwth;
   fieldw -= 3;
   itemp = dpoints - ChrEmantisa;
   back = String;
   ewth = ChrShortString(itemp,String,0);
   if(!ChrEfmtwth) {
       fieldw -= ewth;
       if(cnvflags & CHR_EFMTPLUS) fieldw--;
   }
   else if(itemp < 0) ewth--;
   if(fieldw < nsigdig) {
       dpoints += ChrRoundValue(dspdig,fieldw,nsigdig);
   }
   else fieldw = nsigdig;

   /*--------------------------------------------------------------------------
   ;
   ; Step 6: Insert the decimal point in the E-format display,store the
   ; exponent at the back of the display, and branch to step 9.
   ;
   ;-------------------------------------------------------------------------*/

   ChrShiftRight(dspdig+ChrEmantisa,1,'.');
   dspdig[++fieldw] = (char)(ChrEfmtchr);
   if(dpoints < ChrEmantisa) {
       dpoints = ChrEmantisa - dpoints;
       dspdig[++fieldw] = '-';
   }
   else {
       dpoints -= ChrEmantisa;
       if(cnvflags & CHR_EFMTPLUS)dspdig[++fieldw] = '+';
       else if(ChrEfmtwth) dspdig[++fieldw] = ' ';
   }
   for(itemp = ChrEfmtwth; itemp > ewth; itemp--) dspdig[++fieldw] = '0';
   back = String;
   ewth = ChrShortString(dpoints,String,0);
   while(ewth--) dspdig[++fieldw] = *back++;
   dspdig[++fieldw] = '\0';
   goto ftoa090;

   ftoa070: /*-----------------------------------------------------------------
   ;
   ; Step 7: The value may be displayed in F-format. Calculate the number of
   ; digits to be shown and round down the display or add trailing zeros as
   ; necessary.
   ;
   ;-------------------------------------------------------------------------*/

   if((cnvflags & CHR_TRAILZERO)) fieldw = nsigdig;
   else {
       fieldw = dpoints + ndec;
       if(fieldw < nsigdig) {
           itemp = fieldw;
           if(itemp < 0) itemp = 0;
           if(ChrRoundValue(dspdig,itemp,nsigdig)) {
               if(!fieldw) dspdig[itemp] = '1';
               else dspdig[itemp] = '0';
               dpoints++;
               fieldw++;
           }
       }
       else for(itemp = nsigdig; itemp < fieldw; itemp++)
           dspdig[itemp] = (char)(ChrNonsig); 
       if(fieldw > 0) dspdig[fieldw] = '\0';
       else dspdig[0] = '\0';
   }
   /*--------------------------------------------------------------------------
   ;
   ; Step 8: If needed, place a decimal into the string and if requested to do
   ; so delete any trailing zeros to the right of the decimal, and insert any
   ; commas that may be needed.
   ;
   ;-------------------------------------------------------------------------*/

   while(dpoints < 1) {
       dpoints++;
       ChrShiftRight(dspdig,1,'0');
       fieldw++;
   }
   if(dpoints < fieldw) {
       ChrShiftRight(dspdig+dpoints,1,'.');
       fieldw++;
       back = dspdig+fieldw-1;
       if(cnvflags & CHR_TRAILZERO)
           for(;(*back=='0' || (int)(*back) == ChrNonsig) &&
                               (fieldw > (ndec+dpoints+1)); back--) {
               *back = '\0';
               fieldw--;
           }
       if(*back == '.') {
           if(!(cnvflags & CHR_TRAILDECP)) {
               *back = '\0';
               fieldw--;
           }
       }
       if(!(cnvflags & CHR_LEADZERO)) 
           if(dspdig[0] == '.') {
               ChrShiftRight(dspdig,1,'0');
               fieldw--;
           }
   }
   else if(cnvflags & CHR_TRAILDECP) {
       dspdig[fieldw++] = '.';
       dspdig[fieldw] = '\0';
   }
   if(cnvflags & CHR_COMMAFLG) for(itemp = dpoints-3; itemp > 0; itemp -= 3) {
       ChrShiftRight(dspdig+itemp,1,',');
       fieldw++;
   }
   ftoa090: /*-----------------------------------------------------------------
   ;
   ; Step 9: If the value was negative, insert a leading minus sign. Else if
   ; the value is positive and a leading plus sign is to be used insert the
   ; plus sign character.
   ;
   ;-------------------------------------------------------------------------*/

   if(sign) {
       if(cnvflags & CHR_MINUSPREN) {
           ChrShiftRight(dspdig,1,'(');
           fieldw++;
           dspdig[fieldw++] = ')';
           dspdig[fieldw] = '\0';
       }
       else {
           ChrShiftRight(dspdig,1,'-');
           fieldw++;
       }
   }
   else if(cnvflags & CHR_PLUSSIGN) {
       ChrShiftRight(dspdig,1,ChrPlus);
       fieldw++;
   }
   if(cnvflags & CHR_DOLRSIGN) {
       ChrShiftRight(dspdig,1,'$');
       fieldw++;
   }
   if(cnvflags & CHR_TRAILBLNK) {
       dspdig[fieldw++] = ' ';
       dspdig[fieldw] = '\0';
   }
   /*--------------------------------------------------------------------------
   ;
   ; Step 10: If the field is to be blank filled either on the right or the
   ; left do so.
   ;
   ;-------------------------------------------------------------------------*/

   itemp = width;
   if(itemp < 0) itemp = -itemp;
   if(cnvflags & CHR_ASTERIX) {
       if(itemp > 0 && fieldw > itemp) {
           fieldw = itemp;
           while(itemp) dspdig[--itemp] = '*';
       }
   }
   if(!(cnvflags & CHR_PADBLANK)) {
       while(fieldw < width)
       {
           ChrShiftRight(dspdig,1,ChrLfill);
           fieldw++;
       }
       if(width < 0) while((fieldw+width) < 0) dspdig[fieldw++] = ' ';
   }
   dspdig[fieldw] = '\0';
   return fieldw;
}


/*
;/doc/ ***********************************************************************
;
; FioStorage: Get Character Storage Area
;
; Synopsis of Service:
;
; #include "chrapi.h"                  Character processing interface
;
; char* FioStorage                     Get character storage area
; (
;    int Length                        Length of needed area
; )
;
; Description of Service:
;
; This service maintains a circular buffer area for use by various character
; manipulation application level services that require temporary storage of
; character information. Note that no services within this character processing
; service provider use this service. 
;
; In particular, this service can be used in one of two ways:
;
;   1) to return a semi-permanent pointer to an area of storage that
;      has a specified length.
;
;   2) to free any storage that this service may be using
;
; Note that the total amount of string storage allocated by this service is
; defined by the global variable ChrBufferSize. The algorithm wraps around
; an area of storage whose size is ChrBufferSize, Thus, pointers returned by
; this service will be valid until the entire area is used. In certain cases
; where an application using this runtime system has highly nested calls,
; characters may be overwritten. In such cases, increase the size of the
; global variable ChrBufferSize.
;
; Properties of Service:
;
; Length           The amount of string storage required or a zero if all
;                  storage is to be freed.
;
; Return Value from Service:
;
; A pointer to the requested storage area or a NULL if a request to free the
; storage area was sent.
; 
;/doc/ ************************************************************************
*/
#ifndef platform_h                /* Is this a standalone file */
#define MEMOFUNC                  /* Gives access to memory allocation */
// #include "platform.h"             /* Define the platform hosting this code */
// #include "chrapi.h"               /* Character manipulation Api */
#endif /* platform_h */

#ifdef FPROTOTYPE
char* FioStorage(int Length)
#else
char* FioStorage(Length)
   int Length;                    /* Length of needed area */
#endif
{
   static char* StorageBuffer = NULL;
   static int   CurrentStart = 0;
   char*   StorageArea;

   /*--------------------------------------------------------------------------
   ;
   ; Step 1: If the length property is zero, then simply free the storage
   ; buffer and return a NULL.
   ;
   ;--------------------------------------------------------------------------*/

   if(Length == 0)
   {
      if(StorageBuffer != NULL)
      {
         free(StorageBuffer);
         StorageBuffer = NULL;
         return NULL;
      }
   }

   /*-------------------------------------------------------------------------
   ;
   ; Step 2: The caller needs a storage area. If the buffer has not yet been
   ; allocated, then do so now.
   ;
   ;-------------------------------------------------------------------------*/

   if(StorageBuffer == NULL)
   {
      StorageBuffer = (char*)(getmem(ChrBufferSize));
      if(StorageBuffer == NULL) return NULL;
      CurrentStart = 0;
   }

   /*-------------------------------------------------------------------------
   ;
   ; Step 3: If the requested storage will fit behind the previously allocated
   ; storage, then place it their; else, rewind the buffer and allocate the
   ; storage to the front of the buffer.
   ;
   ;-------------------------------------------------------------------------*/

   if((CurrentStart + Length) > ChrBufferSize) CurrentStart = 0;
   StorageArea = StorageBuffer + CurrentStart;
   CurrentStart += Length;
   return StorageArea;
}

/*
;/doc/ ***********************************************************************
;
; FioPicture: Convert a value via a picture
;
; Synopsis of Service:
;
; #include "chrapi.h"                  Character processing interface
;
; int FioPicture                       Convert a value via a picture
; (
;    double value                      Value to be converted
;    char*  Picture                    Picture describing desired format
;    char*  crec                       Coded record to receive formatted string
; )
;
; Description of Service:
;
; This utility servicen controls the conversion of a double precision value
; to string form under the control of a business formattting descriptor
; string. The length of the string determines the field width of the final
; display. If this width is too small for the number, then the output will
; be a string of asterisks filling the field. Valid characters for the
; string are 
|    6 9 +  -  $  ,  *  Z  #  .  CR  DB B  0 / ()
; The use of these valid characters is explained below:
;
; Char  Use
; ----  ---    
; 6     left-justifies digits with no leading or trailing blanks.
; +     If only the first character is +, then the sign of the number (+ or -)
;       is entered in the leftmost portion of the field (fixed sign). If the
;       string begins with more the one + sign, then these will be replaced
;       by blanks and the sign of the number (+ or -) will be printed in the
;       field position immediately to the left of the first printing character
;       of the number (floating sign). If the rightmost character of the string
;       is +, then the sign of the number (+ or -) will be printed in that
;       field position following the number (trailing sign).
; -     The minus sign behaves the same as a plus sign except that a space
;       (blank) is entered instead of a + if the number is positive (plus sign
;       suppression).
; $     A dollar sign $ may at most be preceded in the string by an optional
;       fixed sign. A single dollar sign will cause a $ to be printed in the
;       corresponding position in the output field (fixed dollar). Multiple
;       dollar signs will be replaced by printing characters in the number, and
;       a single $ will be printed in the position immediately to the left of the
;       leftmost printing character of the number (floating dollar).
; *     Asterisks may be preceded only by an optional fixed sign and/or a fixed
;       dollar. Asterisks in positions used by digits of of the number will be
;       replaced by those digits. The remainder will be printed as asterisks
;       (field filling).
; Z     If the digit corresponding to a Z in the output number is a leading zero,
;       a space (blank) will be printed in that position; otherwise the digit in
;       the number will be printed (leading-zero suppression).
; # 9   The number sign indicates a digit position not subject to leading-zero
;       suppression: the digit in the number will be printed in its corresponding
;       portion whether zero or not (zero nonsuppression).
; .     A decimal point indicates the position of the decimal point in the output
;       number. Only the # sign or 9 and either trailing signs or credit (CR) may
;       follow the decimal point.
; ,     Commas may be placed after any leading character, but before the decimal
;       points. If a significant character of the number (not a sign or dollar)
;       precedes the comma, a comma will be printed in that position. If not
;       preceded by a significant character, a space will be printed in this
;       position unless the comma is in an asterisk field. In that case an * will
;       be printed in that position.
; CR    The characters CR may only be used as the last two (rightmost) characters
;       of the string. If the number is positive, two spaces will be printed
;       following it. If negative, the letters CR will be printed.
; DB    The characters CR may only be used as the last two (rightmost) characters
;       of the string. If the number is positive, two spaces will be printed
;       following it. If negative, the letters CR will be printed.
; B     Blanks, zeros, and slashes may be placed in any position.
;       Blanks, zeros, and slashes may be placed in any position.
; /     Blanks, zeros, and slashes may be placed in any position.
; (n)   Any character may be followed by a constant integer enclosed in
;       parentheses. This has the identical effect of entering that character the
;       indicated number of times.
;
; Properties of Service:
;
; value       Value to be converted
;
; Picture     Picture describing desired format
;
; crec        Coded record to receive formatted string
;
; Return Value from Service:
;
; This service returns the length of the converted form, not including the
; null-value at the end of the string.
;
;/doc/ ************************************************************************
*/
#ifndef platform_h                /* Is this a standalone file */
#define CHARTYPE                  /* Gives access to character type info */
#define RAWMFUNC                  /* Gives access to raw memory services */
#define STRGFUNC                  /* Gives access to the string services */
// #include "platform.h"             /* Define the platform hosting this code */
// #include "chrapi.h"               /* Define character manipulation interface */
#endif /* platform_h */

#ifdef FPROTOTYPE
int FioPicture(double value,char* Picture,char* crec)
#else
int FioPicture(value,Picture,crec)
double value;                     /* Value to be converted */
char* Picture;                    /* Business formatting descriptor string */
char* crec;                       /* Coded record to receive formatted strng */
#endif
{
   char* cntr;               /* Points to current postion in count fld */
   int cwidth;               /* Width of count field specification */
   int dollar;               /* Dollar sign convention flag */
   char dspdig[400];         /* Temporary display buffer */
   char* field;              /* Points to start of field */
   int ichar;                /* Output display starting character */
   int idecp;                /* Position of dec point in output string */
   int iwd;                  /* Number of digits to be displayed */
   int lsign;                /* Leading sign convetion flag */
   int minus;                /* Minus indicator */
   int nc;                   /* Current field character count*/
   int nchar;                /* Number of characters in result display */
   int nd;                   /* Position of decimal point */
   int ndec;                 /* Number of decimal places */
   int plus;                 /* Character to be used for the plus sign */
   int repeat;               /* Repeat count */
   int tsign;                /* Trailing sign convention flag */
   int freefrm;              /* Triggers a free-form display */

   /*--------------------------------------------------------------------------
   ;
   ; Step 1: Scan the string to determine the settings of the key control
   ; variables as follows:
   ;
   ; Name    Description
   ; ----    -----------
   ; iwd     The number of numeric digits allocated. This number must at
   ;         least as large as the number of digits in the number
   ; ndec    The number of decimal places associated with the number
   ;         (-1 = none)
   ; lsign   The leading sign convention code: 0 - no leading sign, 1 -
   ;         fixed sign, -1 - floating sign.
   ; tsign   The trailing sign convention: 0 - no trailing sign, 1 - normal
   ;         trailing sign, -1 - CR trailing sign, -2 - DB trailing sign.
   ; plus    The character to be used for the plus sign.
   ; dollar  The dollar sign convention code: 0 - no dollar sign, 1 - fixed
   ;         dollar sign, -1 - floating dollar sign.
   ; freefrm The display is to be free-form, with no leading or trailing
   ;         blanks;
   ;
   ;------------------------------------------------------------------------*/

   iwd = strlen(Picture);
   idecp = ndec = -1;
   nchar = lsign = tsign = dollar = freefrm = 0;
   plus = ' ';
   cntr = Picture;
   if(*Picture == '+') 
   {
      iwd--;
      plus = '+';
      if(*(Picture+1) == '+' || *(Picture+1) == '(') lsign = -1;
      else lsign = 1;
   }
   if(*Picture == '6') 
   {
      freefrm = 1;
   }
   else if(*Picture == '-') 
   {
      iwd --;
      if(*(Picture+1) == '-' || *(Picture+1) == '(') lsign = -1;
      else lsign = 1;
   }
   else if(*(Picture+iwd-1) == '+') 
   {
      iwd--;
      plus = '+';
      tsign = 1;
   }
   else if(*(Picture+iwd-1) == '-') 
   {
      iwd--;
      tsign = 1;
   }
   else if(*(Picture+iwd-1) == 'r' || *(Picture+iwd-1) == 'R') 
   {
      iwd -= 2;
      tsign = -1;
   }
   else if(*(Picture+iwd-1) == 's' || *(Picture+iwd-1) == 'S') 
   {
      iwd -= 1;
      tsign = -3;
   }
   else if(*(Picture+iwd-2) == 'd' || *(Picture+iwd-2) == 'D') 
   {
      iwd -= 2;
      tsign = -2;
   }
   field = Picture;
   while(*field == '+' || *field == '-') field++;
   if(*field == '$') 
   {
      iwd--;
      if(*(field+1) == '$' || *(field+1) == '(') dollar = -1;
      else dollar = 1;
   }
   while(*field) 
   {
      repeat = 0;
      if(*(field+1) == '(') 
      {
         cntr = field+2;
         repeat = ChrShortFromString(&cntr,0);
         cwidth = cntr - field;
         iwd += (repeat - cwidth - 1);
      }
      if(*field == ',' || *field == 'B' || *field == '.' || 
                          *field == '0' || *field == '/') 
      {
         if(repeat) iwd -= repeat;
         else iwd--;
         if(*field == '.') ndec = 0;
      }
      else if(ndec >= 0 && (*field == '#' || *field == '9' ||
                            *field == 'Z' || *field == '$')) 
      {
         if(repeat) ndec += repeat;
         else ndec++;
      }
      else if(*field == 'D') field++;
      if(repeat) field = cntr + 1;
      else field++;
   }

   /*--------------------------------------------------------------------------
   ;
   ; Step 2: Form the actual numeric display as though it were being
   ; entered under an F format with the specified number of decimal places.
   ;
   ;-------------------------------------------------------------------------*/

   ChrDoubleToChar(value,ChrNsig,&nd,&minus,dspdig);
   field = dspdig;
   nc = nd;
   if(ndec > 0) nc = nd + ndec;
   if(nc < 0) 
   {
      nc = -nc;
      if(nc > 1) minus = 0;
      ChrShiftRight(field,nc,'0');
      nd += nc;
      nc = 0;
      *(field+ChrNsig) = '\0';
   }
   if(!(ChrFlags & CHR_ZDCOBOL) && (nc < ChrNsig)) 
   {
      if(ChrRoundValue(field,nc,ChrNsig)) 
      {
         if(nc != 0 || nd > 0) *(field+nc++) = '0';
         else *(field+nc++) = '1';
         nd++;
      }
   }
   else if(nc > ChrNsig) filmem(field+ChrNsig,nc-ChrNsig,'0');
   *(field+nc) = '\0';
   if(nd < 0) 
   {
      nd = -nd;
      ChrShiftRight(field,nd,'0');
      nc += nd;
      nd = 0;
   }

   /*--------------------------------------------------------------------------
   ;
   ; Step 3: The F-format display of the number of the number has been
   ; formed. Move through the business format string and begin entering
   ; characters into the actual communications record.
   ;
   ;-------------------------------------------------------------------------*/

   if(nc > iwd) 
   {
      if(ChrFlags & CHR_ZDCOBOL) 
      {
         ChrShiftLeft(field,nc-iwd);
         nc = iwd;
      }
      else 
      {
         nd = strlen(Picture);
         filmem(crec+nchar,nd,'*');
         nchar += nd;
         return nchar;
      }
   }
   if(nc < iwd) ChrShiftRight(field,iwd-nc,'0');
   nd = 0;
   ichar = nchar;
   if(lsign > 0) 
   {
      if(minus) crec[nchar++] = '-';
      else crec[nchar++] = (char) plus;
      Picture++;
   }
   else if(lsign < 0) Picture++;
   if(dollar > 0)
   {
      crec[nchar++] = '$';
      Picture++;
   }
   else if(dollar < 0 && *(Picture+1) == '$') Picture++;
   while(*Picture && (*field || *Picture == 'B' || *Picture == '0' ||
                             *Picture == '.' || *Picture == '/')) 
   {
      repeat = 0;
      if(*(Picture+1) == '(') 
      {
         cntr = Picture+2;
         repeat = ChrShortFromString(&cntr,0);
         if(*Picture == '$') repeat--;
      }
      for(;;) 
      {
         if(*Picture == ',') 
         {
            if(nd != 0) nc = ',';
            else if(*(Picture+1) == '*') nc = '*';
            else nc = ' ';
         }
         else if(*Picture == 'B') nc = ' ';
         else if(*Picture == '.' || *Picture == '0' || *Picture == '/') 
         {
            nc = *Picture;
            if(nc == '.') 
            {
               if(dollar < 0 && nd == 0) crec[nchar++] = '$';
               if(lsign < 0 && nd == 0) 
               {
                  if(minus) crec[nchar++] = '-';
                  else crec[nchar++] = (char)(plus);
                  nd = 1;
               }
                  idecp = nchar;
               }
            }
            else if(nd != 0 || *field != '0' || *Picture == '#' || *Picture == '9') 
            {
               nc = *field;
               if(nd == 0 && (*field != '0' || *Picture == '#' || *Picture == '9')) 
               {
                  if(lsign < 0) 
                  {
                     if(minus) crec[nchar++] = '-';
                     else crec[nchar++] = (char) plus;
                  }
                  if(dollar < 0 && idecp < 0) crec[nchar++] = '$';
                  nd = 1;
               }
               field++;
            }
            else 
            {
               if(*Picture == '*') nc = '*';
               else nc = ' ';
               field++;
            }
            crec[nchar++] = (char) nc;
            if(repeat > 1) repeat--;
            else break;
         }
         if(repeat) Picture = cntr+1;
         else Picture++;
      }
      if(nd == 0) 
      {
      if(dollar > 0 && crec[ichar] == '$' && crec[ichar+1] != '*') 
          crec[ichar] = ' ';
      else if(dollar < 0) crec[nchar++] = ' ';
      if(crec[ichar] == '+') crec[ichar] = ' ';
      if(idecp >= 0 && crec[idecp+1] != '*') 
      {
         crec[idecp] = ' ';
         if(idecp > 0 && dollar < 0) crec[idecp-1] = ' ';
      }
      if(lsign < 0 && tsign == 0) crec[nchar++] = ' ';
   }
   else if(idecp >= 0 && crec[idecp+1] == ' ')
   {
      idecp++;
      while(idecp < nchar) 
      {
         if(crec[idecp] == ' ') crec[idecp] = '0';
         else break;
         idecp++;
      }
   }
   if(tsign != 0) 
   {
      if(tsign > 0) 
      {
         if(minus) crec[nchar++] = '-';
         else crec[nchar++] = (char) plus;
      }
      else if(tsign == -1) 
      {
         if(minus) crec[nchar++] = 'C';
         else crec[nchar++] = ' ';
         if(minus) crec[nchar++] = 'R';
         else crec[nchar++] = ' ';
      }
      else if(tsign == -2) 
      {
         if(minus) crec[nchar++] = 'D';
         else crec[nchar++] = ' ';
         if(minus) crec[nchar++] = 'B';
         else crec[nchar++] = ' ';
      }
      else 
      {
         if(minus) crec[nchar++] = '-';
         else crec[nchar++] = ' ';
      }
   }
   if(freefrm)
   {
      crec[nchar] = '\0';
      while(nchar > ichar)
      {
         if(crec[ichar] != ' ') break;
         ChrShiftLeft(crec+ichar,1);
         nchar--;
      }
   }
   return nchar;
}

/*
;/doc/ ***********************************************************************
;
; ChrShiftLeft: Shift character string left
;
; Synopsis of Service:
;
; #include "chrapi.h"                  Character processing interface
;
; void ChrShiftLeft                    Shift character string left
; (
;    char* s,                          String to be shifted
;    int   n                           Amount to shift the string
; )
;
; Description of Service:
;
; This service shifts a null-terminated character string left a specified
; number of positions. The characters that are overwritten are lost.
;
; The most common error in using this service involves forgetting that the
; string must be null-terminated.
;
; Properties of Service:
;
; s         String to be shifted
;
; n         Number of places to be shifted
;
; Return Value from Service: None
;
;/doc/ ************************************************************************
*/

#ifndef platform_h                /* Is this a standalone file */
#define STRGFUNC                  /* Gives access to the string services */
// #include "platform.h"             /* Define the platform hosting this code */
// #include "chrapi.h"               /* Character manipulation Api */
#endif /* platform_h */

#ifdef FPROTOTYPE
void ChrShiftLeft(char* s, int n)
#else
void ChrShiftLeft(s,n)
char* s;                     /* String to be shifted */
int n;                       /* Number of places to be shifted */
#endif
{
   char* b;             /* Dummy pointer to source of move */

    if(n < (int) strlen(s)) for(b = s+n; *b; b++,s++) *s = *b;
    *s = '\0';
}

/*
;/doc/ ***********************************************************************
;
; FncFindFile: Find a File Using the Current List
;
; Synopsis of Service:
;
; #include "fncapi.h"             File naming conventions interface
;
; char* FncFindFile               Find a file using the current list
; (
;    char* FileName               Local filename being sought
;    char* LocalName              Storage for local name
; )
;
; Description of Service:
;
; This service attemps to find the specified file using the current search
; list. If there is no search list specified, then simply look in the current
; directory. The term "pathname" is deliberately avoided here. The names formed
; by this service are literally simply the concatenation of the entries in the
; search list and the filename as passed in.
;
; Properties of Service:
;
; FileName      The name of the file in host platform notation -- i.e.
;               in a form that would yield a successfull open from the
;               operation system if the file is located in the current
;               directory.
;
; LocalName     Supplies storage for the actual names used to open the file.
;               If this service succeeds, then a pointer to the start of this
;               property is in fact returned. If the service fails then this
;               contains the last name attempted.
;
; Return Value from Service:
;
; If the file can be located, then this service returns a pointer to the
; name to be used to open it. If the file is in the current directory then
; the value of 'FileName' is simply retuned. If the file is elsewhere in the
; Search List then a name with the appropriate prefix is returned.
;
; The file cannot be located anywhere on the search list then return a NULL.
;
;/doc/ ************************************************************************
*/
#ifndef platform_h                /* Is this a standalone file */
#define MEMOFUNC                  /* Access to memory allocation service */
#define OSYSFUNC                  /* Gives access to the operating system */
#define PRIOFUNC                  /* Gives access to portable I/O services */
#define RAWMFUNC                  /* Gives access to raw memory operations */
#define STRGFUNC                  /* Gives access to string manipulation */
// #include "platform.h"             /* Define the platform hosting this code */
// #include "fncapi.h"               /* File naming conventions interface */
#endif /* platform_h */

#ifdef FPROTOTYPE
char* FncFindFile(char* FileName,char* LocalName)
#else
char* FncFileFile(FileName,LocalName)
   char* FileName;                /* Local filename being sought */
   char* LocalName;               /* Storage for local name */
#endif
{
   binfile FileHandle;       /* Platform handle for file */
   char*   List;             /* Current position in search list */
   int     NameLength;       /* Length of FileName including nul*/
   int     Length;           /* Length of location name */

   /*--------------------------------------------------------------------------
   ;
   ; Step 1: If there is no Search List specified, then simply see if the file
   ; is in the local directory. If it is then return a pointer to the file name
   ; else return a NULL.
   ;
   ;-------------------------------------------------------------------------*/

   FileHandle = rdobinf(FileName);
   if(binopner(FileHandle) == 0)
   {
      clsbinf(FileHandle);
      return FileName;
   }
   if((List = FncSearchList) == NULL || FileName[0] == FncDirChar) return NULL;
   NameLength = strlen(FileName) + 1;

   ExamineNextLocation: /*----------------------------------------------------
   ;
   ; Step 2: We have a Search List. Examine the curent location and branch
   ; according to whether we are at the end then we have failed to find the
   ; file. Simply return a NULL.
   ;
   ;-------------------------------------------------------------------------*/

   if(!*List) return NULL;

   /*-------------------------------------------------------------------------
   ;
   ; Step 3: If the current search list entry consists of the current directory
   ; character then check the current directory and if the file is present
   ; return a pointer to its name as entered; else move on the examine the
   ; next location in the Search List.
   ;
   ;-------------------------------------------------------------------------*/

   if(*List == '.' && (*(List+1) == '\0' || *(List+1) == FncSepChar))
   {
      List++;
      if(*List == FncSepChar) List++;
      FileHandle = rdobinf(FileName);
      if(binopner(FileHandle)) goto ExamineNextLocation;
      clsbinf(FileHandle);
      return FileName;
   }

   /*-------------------------------------------------------------------------
   ;
   ; Step 4: We are examining some other location. Form its name by
   ; concatenating the location name and the filename. Note that if the
   ; location name does not end in a directory separation character then
   ; we add one ourselves.
   ;
   ;------------------------------------------------------------------------*/

   Length = 0;
   if(*List == '"') List++;
   while(*List != '\0' && *List != FncSepChar)
      LocalName[Length++] = *List++;
   if(LocalName[Length-1] == '"') Length--;
   if(LocalName[Length-1] != FncDirChar) LocalName[Length++] = FncDirChar;
   if(*List == FncSepChar) List++;
   cpymem(FileName,LocalName+Length,NameLength);

   /*-------------------------------------------------------------------------
   ;
   ; Step 5: If the file can be opened in the current location, then return
   ; a pointer to the full location name; else loop back to examine the next
   ; location.
   ;
   ;------------------------------------------------------------------------*/

   FileHandle = rdobinf(LocalName);
   if(binopner(FileHandle)) goto ExamineNextLocation;
   clsbinf(FileHandle);
   return LocalName;
}

#undef three
#undef one
#undef zero

