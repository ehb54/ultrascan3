message(STATUS "US_COMMON ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}")
# include ultrascan code dependencies as interface library
add_library(us_common INTERFACE)

target_include_directories(us_common INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/programs
)
target_link_libraries(us_common INTERFACE
        Qwt::Qwt
        Threads::Threads
        qwtplot3d
        us_utils
        us_gui
        archive
)
target_link_libraries(us_common INTERFACE
        Qt::Core
        Qt::Gui
        Qt::OpenGL
        Qt::Svg
        Qt::Xml
        Qt::Widgets
)
if(NOT "NO_DB" IN_LIST DEFINED)
    target_link_libraries(us_common INTERFACE mysqlclient ssl crypto)
    target_include_directories(us_common INTERFACE /usr/include)
    target_include_directories(us_common INTERFACE /usr/include/mysql)
    target_include_directories(us_common INTERFACE "$ENV{MYSQLPATH}" "$ENV{MYSQLPATH}/include")
endif()
if(UNIX)
    target_compile_definitions(us_common INTERFACE
            INTEL
            LINUX
    )

    target_link_libraries(us_common INTERFACE
            GLU
            X11
    )
endif()

if(WIN32)
    target_compile_definitions(us_common INTERFACE
            INTEL
            QT_SHAREDMEMORY
            QT_SYSTEMSEMAPHORE
            __LCC__
    )

    target_link_libraries(us_common INTERFACE
            c:/msys64/mingw64/x86_64-w64-mingw32/lib/libadvapi32.a
            c:/msys64/mingw64/x86_64-w64-mingw32/lib/libgdi32.a
            c:/msys64/mingw64/x86_64-w64-mingw32/lib/libuser32.a
            c:/msys64/mingw64/x86_64-w64-mingw32/lib/libws2_32.a
            psapi
    )
endif()


if(MACOS)
    target_include_directories(us_common INTERFACE
            ../../Frameworks/QtCore.framework/Headers
            ../../Frameworks/QtGui.framework/Headers
            ../../Frameworks/QtOpenGL.framework/Headers
            ../../Frameworks/QtSvg.framework/Headers
            ../../Frameworks/QtXml.framework/Headers
            ../../qwtplot3d/include
            ../Frameworks/QtCore.framework/Headers
            ../Frameworks/QtGui.framework/Headers
            ../Frameworks/QtOpenGL.framework/Headers
            ../Frameworks/QtSvg.framework/Headers
            ../Frameworks/QtXml.framework/Headers
            ../qwtplot3d/include
            /usr/X11R6/include
            /usr/local/mysql/include
    )

    target_compile_definitions(us_common INTERFACE
            MAC MAC
            OSX OSX
    )

    target_link_libraries(us_common INTERFACE
            "-framework QtOpenGL"
    )
endif()

if(NOT "NO_DB" IN_LIST DEFINED)
add_subdirectory(us)
add_subdirectory(us_2dplot)
add_subdirectory(us_2dsa)
add_subdirectory(us_abde)
add_subdirectory(us_analysis_profile)
add_subdirectory(us_analyte)
add_subdirectory(us_buffer)
endif ()
