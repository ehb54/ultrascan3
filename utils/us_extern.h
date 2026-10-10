/**
 * @file us_extern.h
 * @brief Defines macros for exporting and importing symbols in shared libraries.
 */

#ifndef US_EXTERN_H
#define US_EXTERN_H

#include "us_utils_qt.h"

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

#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__))
#if defined(US_UTIL_STATIC)
  #define US_EXPORT
  #define US_IMPORT
  #define US_UTIL_EXTERN
  #define US_GUI_EXTERN
  #define US_EXTERN
#elif defined(US_MAKE_DLL)
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
#endif

/**
 * @def US_EXPORT
 * @brief Default definition for exporting symbols if not on Windows.
 */
#if !defined(US_EXPORT)
#if defined(__GNUC__) || defined(__clang__)
  #define US_EXPORT       __attribute__((visibility("default")))
#else
  #define US_EXPORT
#endif
#endif

/**
 * @def US_IMPORT
 * @brief Default definition for importing symbols if not on Windows.
 */
#if !defined(US_IMPORT)
  #if defined(__GNUC__) || defined(__clang__)
    #define US_IMPORT __attribute__((visibility("default")))
  #else
    #define US_IMPORT
  #endif
#endif

/**
 * @def US_UTIL_EXTERN
 * @brief Default definition for utility symbols if not on Windows.
 */
#if !defined(US_UTIL_EXTERN)
  #define US_UTIL_EXTERN US_EXPORT
#endif

/**
 * @def US_GUI_EXTERN
 * @brief Default definition for GUI symbols if not on Windows.
 */
#if !defined(US_GUI_EXTERN)
  #define US_GUI_EXTERN US_EXPORT
#endif

/**
 * @def US_CB_STATE_CHANGED
 * @brief Portable signal pointer for "a QCheckBox check state changed".
 *
 * Qt 6.7 introduced QCheckBox::checkStateChanged(Qt::CheckState) and deprecated
 * QCheckBox::stateChanged(int) in 6.9, so builds with
 * QT_DISABLE_DEPRECATED_UP_TO >= 0x060900 no longer see stateChanged. Qt 5 only
 * ever had stateChanged. Both carry the same values, and Qt::CheckState
 * converts implicitly to int, so a slot taking int works with either.
 *
 * Usage: connect( ck_box, US_CB_STATE_CHANGED, this, &US_Foo::bar );
 */
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
  #define US_CB_STATE_CHANGED (&QCheckBox::checkStateChanged)
#else
  #define US_CB_STATE_CHANGED (&QCheckBox::stateChanged)
#endif

#endif // US_EXTERN_H
