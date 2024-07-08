# Specify the library
add_library(utils SHARED
        us_datafiles.cpp
        us_datafiles.h
        )

# Ensure debug information is included
target_compile_options(utils PRIVATE -g -O0)

# Link Qt libraries to the utils library
target_link_libraries(utils Qt6::Core)

# Specify include directories for utils
target_include_directories(utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})



