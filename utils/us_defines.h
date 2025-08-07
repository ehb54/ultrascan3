//! \file us_defines.h
//! \anchor US_Defines
#ifndef US_DEFINES_H
#define US_DEFINES_H

//! The version of UltraScan
#if QT_VERSION > 0x050000
#define US_Version QString("4.0")
#else
#define US_Version QString("3.3")
#endif


//! The directory/key for US_Settings storage
#define US3 QString("UltraScan3")

// Define the platform
#ifdef SPARC
#define PLATFORM "sparc"
#define TITLE "Sun Sparc"
#endif

#if defined(INTEL) || defined(WIN32)
#define PLATFORM "intel"
#define TITLE "Intel"
#endif

#ifdef MAC
#define PLATFORM "mac"
#define TITLE "Macintosh"
#endif

#ifdef OPTERON
#define PLATFORM "opteron"
#define TITLE "64-bit AMD Opteron"
#endif

#ifdef SGI
#define PLATFORM "sgi"
#define TITLE "Silicon Graphics"
#endif

// Define the Operating System
#ifdef WIN32
#define OS "win32"
#define OS_TITLE "Windows"
#endif

#ifdef FREEBSD
#define OS "freebsd"
#define OS_TITLE "FreeBSD"
#endif

#ifdef OPENBSD
#define OS "openbsd"
#define OS_TITLE "OpenBSD"
#endif

#ifdef NETBSD
#define OS "netbsd"
#define OS_TITLE "NetBSD"
#endif

#ifdef LINUX
#define OS "linux"
#define OS_TITLE "Linux"
#endif

#ifdef OSX
#define OS "osx"
#define OS_TITLE "OS-X"
#endif

#ifdef IRIX
#define OS "irix"
#define OS_TITLE "Irix"
#endif

#ifdef SOLARIS
#define OS "solaris"
#define OS_TITLE "Solaris"
#endif

#endif
