//! \file us_defines.h
//! \anchor US_Defines
#ifndef US3_DEFINES_H
#define US3_DEFINES_H

//! Color Palettes for GUI elements QT4/QT3
#ifdef QT4
#include "../include/us_gui_settings.h"
#define PALET_FRAME  QPalette( US_GuiSettings::frameColor()  )
#define PALET_NORMAL QPalette( US_GuiSettings::normalColor() )
#define PALET_EDIT   QPalette( US_GuiSettings::editColor()   )
#define PALET_LABEL  QPalette( US_GuiSettings::labelColor()  )
#define PALET_LISTB  QPalette( US_GuiSettings::editColor()   )
#define PALET_PUSHB  QPalette( US_GuiSettings::pushbColor()  )
#define AUTFBACK(x)  (x)->setAutoFillBackground(true)
#else
#define GLOCOLS USglobal->global_colors
#define PALET3XA(a) QPalette((a),(a),(a))
#define PALET_FRAME  PALET3XA(GLOCOLS.cg_frame)
#define PALET_NORMAL PALET3XA(GLOCOLS.cg_normal)
#define PALET_EDIT   PALET3XA(GLOCOLS.cg_edit)
#define PALET_LABEL  PALET3XA(GLOCOLS.cg_label)
#define PALET_LISTB  PALET3XA(GLOCOLS.cg_normal)
#define PALET_PUSHB  QPalette(GLOCOLS.cg_pushb,GLOCOLS.cg_pushb_disabled,GLOCOLS.cg_pushb_active)
#define AUTFBACK(x)
#endif

//! The version of UltraScan III
#define US3_Version QString("1.0")


//! The directory/key for US_Settings storage
#define US3 QString( "UltraScan3" )

// Define the platform
#ifdef PLATFORM
#undef PLATFORM
#endif
#ifdef TITLE
#undef TITLE
#endif
#ifdef SPARC
  #define PLATFORM "sparc"
  #define TITLE    "Sun Sparc"
#endif

#if defined(INTEL) || defined(WIN32)
  #define PLATFORM "intel"
  #define TITLE    "Intel"
#endif

#ifdef MAC
  #define PLATFORM "mac"
  #define TITLE    "Macintosh"
#endif

#ifdef OPTERON
  #define PLATFORM "opteron"
  #define TITLE    "64-bit AMD Opteron"
#endif

#ifdef SGI
  #define PLATFORM "sgi"
  #define TITLE    "Silicon Graphics"
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
