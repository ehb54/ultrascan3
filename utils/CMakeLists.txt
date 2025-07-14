# Auto-discover source files (excluding subdirectories)
file(GLOB SOURCES "*.cpp")
file(GLOB HEADERS "*.h")

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set_source_files_properties(us_license_t.cpp
            PROPERTIES COMPILE_DEFINITIONS
            "TITLE=\"64-bit AMD Opteron\";OS_TITLE=\"Linux\";PLATFORM=\"opteron\";OS=\"linux\""
            )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set_source_files_properties(us_license_t.cpp
            PROPERTIES COMPILE_DEFINITIONS
            "TITLE=\"Macintosh\";OS_TITLE=\"OSX\";PLATFORM=\"mac\";OS=\"osx\""
            )
elseif(WIN32)
    set_source_files_properties(us_license_t.cpp
            PROPERTIES COMPILE_DEFINITIONS
            "TITLE=\"Intel\";OS_TITLE=\"Windows\";PLATFORM=\"intel\";OS=\"win32\""
            )
endif()

# Create the utils library
add_library(utils ${SOURCES} ${HEADERS})

# Ensure debug information is included
target_compile_options(utils PRIVATE -g -O0)

# Link Qt libraries to the utils library
target_link_libraries(utils
        Qt5::Core
        Qt5::Xml
        Qt5::Sql
        Qt5::Network
        OpenSSL::SSL
        OpenSSL::Crypto
        ${MYSQL_LIBRARIES}
        ${LIBARCHIVE_LIBRARIES}
        )

# Specify include directories for utils
target_include_directories(utils PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${MYSQL_INCLUDE_DIRS}
        ${LIBARCHIVE_INCLUDE_DIRS}
        )

# Add compile flags for MySQL and libarchive
target_compile_options(utils PRIVATE ${MYSQL_CFLAGS_OTHER} ${LIBARCHIVE_CFLAGS_OTHER})



