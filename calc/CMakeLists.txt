# UltraScan3 calculation library (us_calc)
#
# The first module library of the headless service layer:  the reporting
# calculations shared by the desktop programs and the LIMS.  It is plain
# C++17 with no Qt, GUI, database or HTTP dependency, so every build profile
# can build and link it.

file(GLOB CALC_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB CALC_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_library(us_calc STATIC ${CALC_SOURCES} ${CALC_HEADERS})
target_compile_features(us_calc PUBLIC cxx_std_17)
target_include_directories(us_calc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(us_calc PROPERTIES
        POSITION_INDEPENDENT_CODE ON
        AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)

add_library(UltraScan3::Calc ALIAS us_calc)
