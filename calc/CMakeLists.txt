# UltraScan3 calculation library (us_calc)
#
# The reporting calculations shared by the desktop programs.  It uses QtCore
# only (no GUI, database or file access), so every build profile, HPC
# included, can build and link it.

file(GLOB CALC_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB CALC_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_library(us_calc STATIC ${CALC_SOURCES} ${CALC_HEADERS})
target_compile_features(us_calc PUBLIC cxx_std_17)
target_link_libraries(us_calc PUBLIC Qt::Core)
# No floating-point contraction (FMA):  the same doubles on every platform
# (see us_calc_fp.h).  MSVC does not contract under its default /fp:precise.
if(NOT MSVC)
    target_compile_options(us_calc PRIVATE -ffp-contract=off)
endif()
target_include_directories(us_calc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(us_calc PROPERTIES
        POSITION_INDEPENDENT_CODE ON
        AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)

add_library(UltraScan3::Calc ALIAS us_calc)
