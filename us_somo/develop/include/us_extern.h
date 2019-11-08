#ifndef US_EXTERN_H
#define US_EXTERN_H
 
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define US_EXPORT       __declspec(dllexport)
#define US_IMPORT       __declspec(dllimport)
#if defined(US_MAKE_DLL)
#define US_EXTERN       US_EXPORT
#elif defined(US_USE_DLL)
#define US_EXTERN       US_IMPORT
#endif
#endif
 
#if !defined(US_EXPORT)
#define US_EXPORT
#endif
 
#if !defined(US_IMPORT)
#define US_IMPORT
#endif
 
#if !defined(US_EXTERN)
#define US_EXTERN
#endif
 
#endif

