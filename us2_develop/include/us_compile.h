#ifndef US_COMPILE
#define US_COMPILE

#  if defined(Q_OS_WIN32)
#    if defined(US_DLL)
#      define US_EXPORT __declspec(dllexport)
#    else
#      define US_EXPORT __declspec(dllimport)
#    endif
#  else
#    define US_EXPORT
#  endif
#endif

