# UltraScan3 calculation library (us_calc), qmake build
#
# Plain C++17 without Qt, GUI, database or HTTP dependencies (see
# CMakeLists.txt).  Built as a static library into ../lib like the other
# UltraScan libraries; library.pri is not used because it adds Qt and the
# database and archive libraries.

exists( ../local.pri ) {
   include( ../local.pri )
}

TEMPLATE     = lib
TARGET       = us_calc
CONFIG      += staticlib c++17 warn_on $$DEBUGORRELEASE
CONFIG      -= qt
DESTDIR      = ../lib
OBJECTS_DIR  = ./obj

HEADERS      = us_calc_matching.h   \
               us_calc_range.h      \
               us_calc_replicate.h  \
               us_calc_tolerance.h  \
               us_calc_types.h      \
               us_calc_units.h      \
               us_calc_validation.h \
               us_calc_version.h

SOURCES      = us_calc_matching.cpp   \
               us_calc_range.cpp      \
               us_calc_replicate.cpp  \
               us_calc_tolerance.cpp  \
               us_calc_units.cpp      \
               us_calc_validation.cpp \
               us_calc_version.cpp
