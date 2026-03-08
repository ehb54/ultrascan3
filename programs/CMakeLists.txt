# UltraScan3 Application Programs
#
# Two build paths:
#
#   APP (US3_NO_DB=OFF):
#     Full programs suite — all us_* programs, links Qt GUI stack + DB + usutils + usgui.
#
#   HPC (US3_NO_DB=ON):
#     Minimal headless path — only us_mpi_analysis, links Qt Core/Network/Xml/Sql + usutils.
#     No GUI modules, no desktop launchers, no help daemon.

cmake_minimum_required(VERSION 3.16)

# Enable Qt's automatic MOC, UIC, and RCC processing
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

include_directories(${CMAKE_BINARY_DIR})

# --------------------------------------------------------------------------
# Common interface for all UltraScan programs
# --------------------------------------------------------------------------
add_library(us_program_common INTERFACE)

target_include_directories(us_program_common INTERFACE
        ${CMAKE_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
        )

# --------------------------------------------------------------------------
# HPC path: minimal deps — Qt Core/Network/Xml/Sql + usutils only
# --------------------------------------------------------------------------
if(US3_NO_DB)

    message(STATUS "[programs] HPC/NO_DB path: building us_mpi_analysis only")

    target_link_libraries(us_program_common INTERFACE
            Qt::Core
            Qt::Network
            Qt::Xml
            Qt::Sql
            )

    # Resources (static archive, needed for UltraScan data initialization)
    if(TARGET UltraScan3::Resources)
        if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.24)
            target_link_libraries(us_program_common INTERFACE
                    $<LINK_LIBRARY:WHOLE_ARCHIVE,UltraScan3::Resources>
            )
        else()
            if(APPLE)
                target_link_options(us_program_common INTERFACE
                        "-Wl,-force_load,$<TARGET_FILE:UltraScan3::Resources>"
                )
                target_link_libraries(us_program_common INTERFACE UltraScan3::Resources)
            else()
                target_link_libraries(us_program_common INTERFACE
                        "-Wl,--whole-archive" UltraScan3::Resources "-Wl,--no-whole-archive"
                )
            endif()
        endif()
    else()
        message(WARNING "[programs] UltraScan3::Resources not found — HPC build may be incomplete")
    endif()

    # Utils: prefer static for HPC (US3_PREFER_STATIC is ON in HPC profile)
    if(US3_PREFER_STATIC AND TARGET UltraScan3::Utils_Static)
        message(STATUS "[programs] HPC: using static Utils")
        add_dependencies(us_program_common us_utils_static)
        target_link_libraries(us_program_common INTERFACE UltraScan3::Utils_Static)
    elseif(TARGET UltraScan3::Utils)
        message(STATUS "[programs] HPC: using shared Utils")
        add_dependencies(us_program_common us_utils)
        target_link_libraries(us_program_common INTERFACE UltraScan3::Utils)
    else()
        message(FATAL_ERROR "[programs] HPC: UltraScan3::Utils not found")
    endif()

    if(UNIX AND NOT APPLE)
        target_link_libraries(us_program_common INTERFACE OpenSSL::Crypto)
    endif()

    # HPC: build only us_mpi_analysis
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/us_mpi_analysis/CMakeLists.txt")
        add_subdirectory(us_mpi_analysis)
        message(STATUS "[programs] HPC: added us_mpi_analysis")
    else()
        message(WARNING "[programs] HPC: us_mpi_analysis/CMakeLists.txt not found")
    endif()

    return()
endif()

# --------------------------------------------------------------------------
# APP path (US3_NO_DB=OFF): full programs suite
# --------------------------------------------------------------------------

# 1) Always-needed deps
target_link_libraries(us_program_common INTERFACE
        Qt::Core
        Qt::Gui
        Qt::Widgets
        Qt::Network
        Qt::Sql
        Qt::Xml
        Qt::Svg
        Qt::OpenGL
        Qt::PrintSupport
        Qwt::Qwt
        qwtplot3d::qwtplot3d
        )
if (USE_QT6)
    target_link_libraries(us_program_common INTERFACE Qt::OpenGLWidgets)
endif()

# resources are their own target at the top level
if(TARGET UltraScan3::Resources)
    if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.24)
        target_link_libraries(us_program_common INTERFACE
                $<LINK_LIBRARY:WHOLE_ARCHIVE,UltraScan3::Resources>
        )
    else()
        if(MSVC)
            target_link_options(us_program_common INTERFACE
                    "/WHOLEARCHIVE:$<TARGET_FILE:UltraScan3::Resources>"
            )
            target_link_libraries(us_program_common INTERFACE UltraScan3::Resources)
        elseif(APPLE)
            target_link_options(us_program_common INTERFACE
                    "-Wl,-force_load,$<TARGET_FILE:UltraScan3::Resources>"
            )
            target_link_libraries(us_program_common INTERFACE UltraScan3::Resources)
        else()
            target_link_libraries(us_program_common INTERFACE
                    "-Wl,--whole-archive" UltraScan3::Resources "-Wl,--no-whole-archive"
            )
        endif()
    endif()
else()
    message(FATAL_ERROR "UltraScan3::Resources target not found – resources must be built before programs/")
endif()

if(UNIX AND NOT APPLE)
    target_link_libraries(us_program_common INTERFACE OpenSSL::Crypto)
endif()

# 2) Utils
if(US3_PREFER_STATIC)
    if(TARGET UltraScan3::Utils_Static)
        message(STATUS "[programs] Using STATIC Utils (US3_PREFER_STATIC=ON)")
        add_dependencies(us_program_common us_utils_static)
        target_link_libraries(us_program_common INTERFACE UltraScan3::Utils_Static)
    elseif(TARGET UltraScan3::Utils)
        message(WARNING "[programs] US3_PREFER_STATIC=ON but no Utils_Static; falling back to shared")
        add_dependencies(us_program_common us_utils)
        target_link_libraries(us_program_common INTERFACE UltraScan3::Utils)
    else()
        message(FATAL_ERROR "[programs] Neither UltraScan3::Utils_Static nor UltraScan3::Utils found")
    endif()
else()
    if(TARGET UltraScan3::Utils)
        message(STATUS "[programs] Using SHARED Utils")
        add_dependencies(us_program_common us_utils)
        target_link_libraries(us_program_common INTERFACE UltraScan3::Utils)
    else()
        message(FATAL_ERROR "[programs] UltraScan3::Utils not found")
    endif()
endif()

# 3) GUI
if(US3_PREFER_STATIC)
    if(TARGET UltraScan3::Gui_Static)
        message(STATUS "[programs] Using STATIC GUI (US3_PREFER_STATIC=ON)")
        add_dependencies(us_program_common us_gui_static)
        target_link_libraries(us_program_common INTERFACE UltraScan3::Gui_Static)
    else()
        message(FATAL_ERROR "[programs] US3_PREFER_STATIC=ON but UltraScan3::Gui_Static not found")
    endif()
else()
    if(TARGET UltraScan3::Gui)
        message(STATUS "[programs] Using SHARED GUI (APP build)")
        add_dependencies(us_program_common us_gui)
        target_link_libraries(us_program_common INTERFACE UltraScan3::Gui)
    elseif(TARGET UltraScan3::Gui_Static)
        message(STATUS "[programs] Augmenting with STATIC GUI to pick up legacy symbols")
        add_dependencies(us_program_common us_gui_static)
        target_link_libraries(us_program_common INTERFACE UltraScan3::Gui_Static)
    else()
        message(FATAL_ERROR "[programs] GUI requested but UltraScan3::Gui not found. \
        Make sure add_subdirectory(gui) is called before add_subdirectory(programs).")
    endif()
endif()

if(WIN32)
    target_compile_definitions(us_program_common INTERFACE QWT_DLL QT_DLL)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        if(MINGW)
            target_link_options(us_program_common INTERFACE "-mwindows")
        elseif(MSVC)
            target_link_options(us_program_common INTERFACE "/SUBSYSTEM:WINDOWS" "/ENTRY:mainCRTStartup")
        endif()
    endif()
endif()

# --------------------------------------------------------------------------
# 4) Add all per-program subdirectories (APP path)
# --------------------------------------------------------------------------
if(NOT ENABLE_MPI)
    set(EXCLUDED_PROGRAMS us_mpi_analysis CMakeFiles generated)
else()
    set(EXCLUDED_PROGRAMS CMakeFiles generated)
endif()

file(GLOB program_dirs LIST_DIRECTORIES true RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*")

set(added 0)
foreach(prog_name ${program_dirs})
    if(NOT IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${prog_name}")
        continue()
    endif()
    if(prog_name IN_LIST EXCLUDED_PROGRAMS)
        continue()
    endif()
    if(prog_name MATCHES "^\\.")
        continue()
    endif()

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${prog_name}/CMakeLists.txt")
        add_subdirectory(${prog_name})
        math(EXPR added "${added} + 1")
        message(STATUS "Programs: using per-directory CMakeLists.txt for ${prog_name}")
    else()
        message(WARNING "Programs: ${prog_name} has no CMakeLists.txt (skipped)")
    endif()
endforeach()

message(STATUS "Programs: added ${added} program directories")
