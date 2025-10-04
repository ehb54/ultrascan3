/**
 * @file us_extern.h
 * @brief Defines macros for exporting and importing symbols in shared libraries.
 */

#ifndef US_EXTERN_H
#define US_EXTERN_H

/**
 * @def US_EXPORT
 * @brief Macro for exporting symbols in a shared library.
 *
 * Defined as __declspec(dllexport) on Windows.
 */

/**
 * @def US_IMPORT
 * @brief Macro for importing symbols from a shared library.
 *
 * Defined as __declspec(dllimport) on Windows.
 */

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
  #define US_EXPORT       __declspec(dllexport)
  #define US_IMPORT       __declspec(dllimport)

  /**
   * @def US_UTIL_EXTERN
   * @brief Macro for exporting or importing utility symbols.
   *
   * Defined as US_EXPORT if US_MAKE_UTIL_DLL is defined, otherwise defined as US_IMPORT.
   */
  #if defined(US_MAKE_UTIL_DLL)
    #define US_UTIL_EXTERN       US_EXPORT
  #else
    #define US_UTIL_EXTERN       US_IMPORT
  #endif

  /**
   * @def US_GUI_EXTERN
   * @brief Macro for exporting or importing GUI symbols.
   *
   * Defined as US_EXPORT if US_MAKE_GUI_DLL is defined, otherwise defined as US_IMPORT.
   */
  #if defined(US_MAKE_GUI_DLL)
    #define US_GUI_EXTERN       US_EXPORT
  #else
    #define US_GUI_EXTERN       US_IMPORT
  #endif

  /**
   * @def US_EXTERN
   * @brief Macro for exporting or importing general symbols.
   *
   * Defined as US_EXPORT if US_MAKE_DLL is defined, otherwise defined as US_IMPORT.
   */
  #if defined(US_MAKE_DLL)
    #if !defined(US_GUI_EXTERN)
      #define US_GUI_EXTERN     US_EXPORT
    #endif
    #if !defined(US_UTIL_EXTERN)
      #define US_UTIL_EXTERN    US_EXPORT
    #endif
    #define US_EXTERN           US_EXPORT
  #else
    #if !defined(US_GUI_EXTERN)
      #define US_GUI_EXTERN     US_IMPORT
    #endif
    #if !defined(US_UTIL_EXTERN)
      #define US_UTIL_EXTERN    US_IMPORT
    #endif
    #define US_EXTERN           US_IMPORT
  #endif
#endif

/**
 * @def US_EXPORT
 * @brief Default definition for exporting symbols if not on Windows.
 */
#if !defined(US_EXPORT)
  #define US_EXPORT
#endif

/**
 * @def US_IMPORT
 * @brief Default definition for importing symbols if not on Windows.
 */
#if !defined(US_IMPORT)
  #define US_IMPORT
#endif

/**
 * @def US_UTIL_EXTERN
 * @brief Default definition for utility symbols if not on Windows.
 */
#if !defined(US_UTIL_EXTERN)
  #define US_UTIL_EXTERN
#endif

/**
 * @def US_GUI_EXTERN
 * @brief Default definition for GUI symbols if not on Windows.
 */
#if !defined(US_GUI_EXTERN)
  #define US_GUI_EXTERN
#endif

#endif // US_EXTERN_H
