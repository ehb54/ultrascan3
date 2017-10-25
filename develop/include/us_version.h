#ifndef US_VERSION_H
#define US_VERSION_H

#if QT_VERSION < 0x040000
# define US_Version_string "9.9"
# define WIN32Version      "-WIN32-9.9"
#else
# define US_Version_string "2.2"
# define WIN32Version      "-WIN32-2.2"
# define SOMO_Revision     "SOMOsvn-2697"
#endif

#if QT_VERSION >= 0x050000
# undef US_Version_string
# define US_Version_string "3.5"
#endif

#endif

