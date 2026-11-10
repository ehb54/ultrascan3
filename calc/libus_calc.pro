# UltraScan3 calculation library (us_calc), qmake build
#
# QtCore only, without GUI or database dependencies (see CMakeLists.txt).
# Built as a static library into ../lib like the other UltraScan
# libraries; library.pri is not used because it adds the GUI, database and
# archive libraries.

exists( ../local.pri ) {
   include( ../local.pri )
}

TEMPLATE     = lib
TARGET       = us_calc
CONFIG      += staticlib c++17 warn_on qt $$DEBUGORRELEASE
QT           = core
DESTDIR      = ../lib
OBJECTS_DIR  = ./obj

# No floating-point contraction (FMA); see us_calc_fp.h
!win32-msvc*: QMAKE_CXXFLAGS += -ffp-contract=off

HEADERS      = $$files(us_calc_*.h)

SOURCES      = $$files(us_calc_*.cpp)
