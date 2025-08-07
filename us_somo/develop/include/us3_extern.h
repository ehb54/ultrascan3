#ifndef US3_EXTERN_H
#define US3_EXTERN_H

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define US_EXPORT __declspec(dllexport)
#define US_IMPORT __declspec(dllimport)

#if defined(US_MAKE_UTIL_DLL)
#define US_UTIL_EXTERN US_EXPORT
#else
#define US_UTIL_EXTERN US_IMPORT
#endif

#if defined(US_MAKE_GUI_DLL)
#define US_GUI_EXTERN US_EXPORT
#else
#define US_GUI_EXTERN US_IMPORT
#endif

#if defined(US_MAKE_DLL)
#if !defined(US_GUI_EXTERN)
#define US_GUI_EXTERN US_EXPORT
#endif
#if !defined(US_UTIL_EXTERN)
#define US_UTIL_EXTERN US_EXPORT
#endif
#define US_EXTERN US_EXPORT
#else
#if !defined(US_GUI_EXTERN)
#define US_GUI_EXTERN US_IMPORT
#endif
#if !defined(US_UTIL_EXTERN)
#define US_UTIL_EXTERN US_IMPORT
#endif
#define US_EXTERN US_IMPORT
#endif
#endif

#if !defined(US_EXPORT)
#define US_EXPORT
#endif

#if !defined(US_IMPORT)
#define US_IMPORT
#endif

#if !defined(US_UTIL_EXTERN)
#define US_UTIL_EXTERN
#endif

#if !defined(US_GUI_EXTERN)
#define US_GUI_EXTERN
#endif

#endif
