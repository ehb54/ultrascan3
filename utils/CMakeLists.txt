# Specify the library
add_library(utils SHARED
        us_ana_profile.cpp
        us_ana_profile.h
        us_datafiles.cpp
        us_datafiles.h
        us_math2.cpp
        us_math2.h
        us_report_gmp.cpp
        us_report_gmp.h
        us_util.cpp
        us_util.h
        us_constants.cpp
        us_constants.h
        us_crc.cpp
        us_crc.h
        us_crypto.cpp us_crypto.h
        us_matrix.cpp us_matrix.h us_crc.h
        us_dataIO.cpp us_dataIO.h
        us_settings.cpp us_settings.h)

# Ensure debug information is included
target_compile_options(utils PRIVATE -g -O0)

# Link Qt libraries to the utils library
target_link_libraries(utils Qt5::Core Qt5::Xml OpenSSL::SSL OpenSSL::Crypto)

# Specify include directories for utils
target_include_directories(utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})



