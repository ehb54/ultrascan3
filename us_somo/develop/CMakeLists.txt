# =============================================================================
# us_somo / US-SOMO  —  CMake build
# =============================================================================
# Builds the SOMO hydrodynamics / SAXS suite. Two classes of application:
#   GUI apps — a single main.cpp that LINKS libus_somo (+ qwt):
#     libus_somo          shared library (SONAME .10) — the bulk of the code
#     us3_somo            main GUI application     (us3_hydrodyn/main.cpp)
#     us_admin            admin GUI                (us_admin/)
#     us3_config          config GUI               (us3_config/)
#   Standalone tools — compile a curated ../src subset directly, NO lib, NO qwt:
#     us_saxs_cmds_t      SAXS command-line tool   (us_saxs_cmds_t/)
#   (us_cluster_server_t dropped: needs Qt3Support / q3socket.h, gone since Qt5)
# Optional (default OFF):
#     us_saxs_cmds_mpi        MPI SAXS tool            SOMO_ENABLE_MPI
#     us_saxs_cmds_mpi_cuda   MPI+CUDA SAXS tool       SOMO_ENABLE_MPI + SOMO_ENABLE_CUDA
#     CUDA code paths in the lib                       SOMO_ENABLE_CUDA
#     ADOLC autodiff                                   SOMO_ENABLE_ADOLC
#
# KEY FACT (verified): SOMO is SELF-CONTAINED. It VENDORS its own fork of the
# us_utils sources (src/us_math.cpp, us_matrix.cpp, us_saxs_util*.cpp, ...) and
# does NOT link UltraScan3::Utils. Its only couplings are third-party: Qt5/Qt6,
# Qwt, OpenGL, zlib, OpenSSL. Runtime ldd of the deployed us3_somo confirms:
# libus_somo.so.10 + qwt + Qt + system libs, no libus_utils.so.
#
# DUAL-MODE: this file configures standalone
#     cmake -S us_somo/develop -B build --toolchain <vcpkg>/scripts/buildsystems/vcpkg.cmake
# OR when pulled in from the root UltraScan3 build via add_subdirectory(us_somo).
#
# In standalone mode the third-party deps come from this directory's own
# vcpkg.json (+ vcpkg-configuration.json), a minimal manifest that reuses the
# repo-root overlay-ports (qwt / qt5-multimedia) and the same
# builtin-baseline as the root. It has qt5 (default) and qt6 features — select
# Qt6 with: --x-feature=qt6 (or -DVCPKG_MANIFEST_FEATURES=qt6 -DVCPKG_MANIFEST_NO_DEFAULT_FEATURES=ON).
# When built from the root instead, the root vcpkg.json (qt5-app / qt6-app
# features) already provides every dep SOMO needs, and this manifest is simply
# ignored (vcpkg reads the manifest at the top-level source).
#
# SOURCE LISTS come from the qmake files at configure time (libus_somo.pro and
# each app's .pro, via cmake/SomoQmakeSources.cmake), so qmake and CMake build
# the same files on every branch while both build systems are in use.
# =============================================================================

# --- standalone bootstrap (skipped when included via add_subdirectory) --------
if(NOT DEFINED PROJECT_NAME)
    cmake_minimum_required(VERSION 3.21)
    project(us_somo VERSION 10.0.0 LANGUAGES CXX C)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    set(SOMO_STANDALONE ON)
else()
    set(SOMO_STANDALONE OFF)   # inherit output dirs from root
endif()

# SOMO requires C++17 in BOTH modes. The root build sets a lower global standard
# (CMAKE_CXX_STANDARD 14), so pin it for this subtree (directory scope → applies
# to all SOMO targets defined here).
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# The root build enables LTO globally (CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE).
# LTO on the shared libus_somo drops symbols the GUI apps link against (undefined
# references to US_Register/US_Config/us_tr/...). SOMO builds cleanly without LTO
# (as standalone, and as its qmake build always did), so disable it for this
# subtree (directory scope; root's other components keep LTO).
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION OFF)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE OFF)

# The root sets -fvisibility=hidden globally (admin/cmake/platform-config.cmake),
# but SOMO's classes are not annotated for export, so that hides ALL of
# libus_somo's symbols and the GUI apps fail to link (undefined US_Register/
# US_Admin/US_Config/...). SOMO builds with default visibility (as standalone and
# under qmake); restore it for this subtree — a later -fvisibility flag wins.
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-fvisibility=default)
endif()

# Qt codegen — scoped to this subtree only (root already sets these globally,
# but standalone mode needs them too).
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# --- options -----------------------------------------------------------------
option(SOMO_ENABLE_MPI   "Build the SOMO MPI SAXS tool (us_saxs_cmds_mpi)" OFF)
option(SOMO_ENABLE_CUDA  "Build SOMO CUDA SAXS kernels"                    OFF)
option(SOMO_ENABLE_ADOLC "Build with ADOLC automatic differentiation"     OFF)

set(SOMO_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

# us_register.cpp derives PLATFORM/TITLE from an arch macro (its own #if chain
# sets the strings right before use). Define INTEL on Linux so that #if block
# fires. This is needed in BOTH standalone and root builds: the string-valued
# -DPLATFORM="intel"/-DTITLE="Intel" that add_compile_definitions() emits does
# NOT survive CMake's quote-escaping into a usable macro (this bites the root's
# own line 209 definition too — it just never mattered because only
# us_register.cpp uses PLATFORM as a bare macro). The bareword INTEL is immune,
# and since the parent's PLATFORM/TITLE don't actually take effect there is no
# redefinition clash. APPLE is covered by MAC, Windows by WIN32.
if(UNIX AND NOT APPLE)
    add_compile_definitions(INTEL)
endif()

# --- dependencies (all third-party) ------------------------------------------
# Qt5 AND Qt6: PR #335 made the SOMO code compatible with both; CI/CodeQL test
# both, and the qmake build is Qt5 today with a planned move to Qt6. Link
# Qt${QT_VERSION_MAJOR}::... throughout, with the major version chosen as:
#   - root build: the root's choice (it sets QT_VERSION_MAJOR from USE_QT6).
#     Re-detecting here could pick the other Qt when both are installed, and
#     SOMO would then build against a different Qt than the rest of UltraScan.
#   - standalone with -DUSE_QT6=ON|OFF: as asked.
#   - standalone otherwise: Qt6 if it has every component SOMO needs, else Qt5.
#     (A bare Qt6 qtbase is not enough: vcpkg's Qt5 builds install one as the
#     host tool that builds qwt.)
# NOTE: Multimedia is required (sounds/). The root manifest's qt5-app/qt6-app
# features include qt5-multimedia / qtmultimedia, and the standalone vcpkg.json
# provides both via its qt5 / qt6 features.
# TODO(Qt6): QtOpenGL split in Qt6 — if the code uses QOpenGLWidget, add the
# OpenGLWidgets component for QT_VERSION_MAJOR==6.
set(SOMO_QT_COMPONENTS Core Gui Widgets Network Xml Svg PrintSupport OpenGL Concurrent Multimedia)
if(NOT SOMO_STANDALONE AND DEFINED QT_VERSION_MAJOR)
    # root build: keep the root's QT_VERSION_MAJOR
elseif(DEFINED USE_QT6)
    if(USE_QT6)
        set(QT_VERSION_MAJOR 6)
    else()
        set(QT_VERSION_MAJOR 5)
    endif()
else()
    find_package(Qt6 QUIET COMPONENTS ${SOMO_QT_COMPONENTS})
    if(Qt6_FOUND)
        set(QT_VERSION_MAJOR 6)
    else()
        set(QT_VERSION_MAJOR 5)
    endif()
endif()
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS ${SOMO_QT_COMPONENTS})
message(STATUS "SOMO: building against Qt${QT_VERSION_MAJOR} (${Qt${QT_VERSION_MAJOR}_VERSION})")
find_package(OpenGL REQUIRED)
find_package(ZLIB    REQUIRED)
find_package(OpenSSL REQUIRED)

# Qwt — reuse root's resolver/alias if we were add_subdirectory'd; otherwise
# resolve here (vcpkg 'unofficial-qwt', or system via admin/cmake/FindQwt.cmake).
if(NOT TARGET Qwt::Qwt)
    find_package(unofficial-qwt CONFIG QUIET)
    if(unofficial-qwt_FOUND AND NOT TARGET Qwt::Qwt)
        add_library(Qwt::Qwt ALIAS unofficial::qwt::qwt)
    endif()
endif()
if(NOT TARGET Qwt::Qwt)
    message(FATAL_ERROR "SOMO: Qwt not found (need a Qwt::Qwt target)")
endif()

# SOMO does `#include <qwt_counter.h>` (flat, like the qmake QWTPATH/include
# layout), but vcpkg nests the headers under include/qwt/. The Qwt::Qwt target's
# interface include (the vcpkg include root) is therefore not enough — add the
# directory that actually contains the qwt headers.
find_path(SOMO_QWT_INCLUDE_DIR NAMES qwt_counter.h PATH_SUFFIXES qwt qwt6 qt5/qwt)
if(NOT SOMO_QWT_INCLUDE_DIR)
    message(FATAL_ERROR "SOMO: qwt headers (qwt_counter.h) not found on the include path")
endif()
message(STATUS "SOMO: qwt headers at ${SOMO_QWT_INCLUDE_DIR}")

# No QwtPlot3D: no SOMO source includes a qwt3d header, and the 3dplot/ entries
# in libus_somo.pro are commented out, so neither the SOMO-local 3dplot/ tree
# nor the root's qwtplot3d::qwtplot3d is linked (nor was it under qmake).

# --- generated version headers -----------------------------------------------
# SOMO generates two headers via shell scripts (as in the qmake build):
#   version.sh  -> include/us_version.h   (needs $us3      = repo root; reads utils/us_defines.h)
#   revision.sh -> include/us_revision.h  (needs $ULTRASCAN = repo root; git log)
# us_config.cpp includes us_version.h, and many src/ files include us_revision.h,
# so both must exist before compilation. Both need git, and both rewrite their
# header only when its contents change.
# On main both scripts are tracked and the headers are untracked build output.
# somo-dev instead commits both headers (and has dropped revision.sh). A build
# must not rewrite a tracked file, so a committed header is used as is, and a
# script runs only for a header git does not track.
# TODO: these write into the SOURCE tree and shell out to git — prefer a
# CMake-native generator writing into ${CMAKE_CURRENT_BINARY_DIR} (see
# admin/cmake/GenerateVersion.cmake).
get_filename_component(SOMO_REPO_ROOT "${SOMO_ROOT}/../.." ABSOLUTE)
find_package(Git QUIET)
set(SOMO_GENHEADER_COMMANDS "")
set(SOMO_GENHEADER_BYPRODUCTS "")
foreach(kind version revision)
    set(_hdr include/us_${kind}.h)
    set(_rc 1)
    if(GIT_FOUND)
        execute_process(COMMAND ${GIT_EXECUTABLE} ls-files --error-unmatch ${_hdr}
                        WORKING_DIRECTORY ${SOMO_ROOT}
                        RESULT_VARIABLE _rc OUTPUT_QUIET ERROR_QUIET)
    endif()
    if(_rc EQUAL 0)
        message(STATUS "SOMO: using the committed ${_hdr}")
    elseif(EXISTS ${SOMO_ROOT}/${kind}.sh)
        list(APPEND SOMO_GENHEADER_COMMANDS
            COMMAND ${CMAKE_COMMAND} -E env "us3=${SOMO_REPO_ROOT}" "ULTRASCAN=${SOMO_REPO_ROOT}" sh ${kind}.sh)
        list(APPEND SOMO_GENHEADER_BYPRODUCTS ${SOMO_ROOT}/${_hdr})
    elseif(EXISTS ${SOMO_ROOT}/${_hdr})
        message(STATUS "SOMO: no ${kind}.sh; using the existing ${_hdr}")
    else()
        message(FATAL_ERROR "SOMO: neither ${kind}.sh nor ${_hdr} exists")
    endif()
endforeach()
if(SOMO_GENHEADER_COMMANDS)
    add_custom_target(us_somo_genheaders ALL
        ${SOMO_GENHEADER_COMMANDS}
        WORKING_DIRECTORY ${SOMO_ROOT}
        BYPRODUCTS ${SOMO_GENHEADER_BYPRODUCTS}
        COMMENT "Generating SOMO version headers")
else()
    add_custom_target(us_somo_genheaders)   # nothing to generate
endif()

# --- the library: libus_somo -------------------------------------------------
# Source/header lists are libus_somo.pro's SOURCES / HEADERS, read at configure
# time — NOT a glob of src/*.cpp. Globbing is wrong here: several src/ TUs are
# app-specific and must not be in the lib, and the lib also pulls in
# shd_mpi/shs_use.cpp. The .pro lists the generated us_revision.h, which
# us_somo_genheaders produces, so it is left out here.
include(${SOMO_ROOT}/cmake/SomoQmakeSources.cmake)
somo_qmake_sources(${SOMO_ROOT}/libus_somo.pro SOMO_LIB_SOURCES SOMO_LIB_HEADERS
    EXCLUDE "/include/us_(revision|version)\\.h$")
list(LENGTH SOMO_LIB_SOURCES _n_src)
list(LENGTH SOMO_LIB_HEADERS _n_hdr)
message(STATUS "SOMO: libus_somo.pro lists ${_n_src} sources, ${_n_hdr} headers")

add_library(us_somo SHARED ${SOMO_LIB_SOURCES} ${SOMO_LIB_HEADERS})
add_dependencies(us_somo us_somo_genheaders)
set_target_properties(us_somo PROPERTIES VERSION 10.0.0 SOVERSION 10)  # libus_somo.so.10

# grpy/ is the header-only GRPY driver module (libus_somo.pro: INCLUDEPATH += $$PWD/grpy)
target_include_directories(us_somo PUBLIC
    ${SOMO_ROOT}/include ${SOMO_ROOT}/src ${SOMO_ROOT}/grpy ${SOMO_QWT_INCLUDE_DIR})

target_compile_definitions(us_somo PUBLIC NO_DB THREAD NO_EDITOR_PRINT)
if(UNIX AND NOT APPLE)
    target_compile_definitions(us_somo PUBLIC UNIX LINUX BIN64)
elseif(APPLE)
    target_compile_definitions(us_somo PUBLIC OSX MAC)
elseif(WIN32)
    target_compile_definitions(us_somo PUBLIC MINGW WIN32)
endif()

target_link_libraries(us_somo PUBLIC
    Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Gui Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::Xml Qt${QT_VERSION_MAJOR}::Svg
    Qt${QT_VERSION_MAJOR}::PrintSupport Qt${QT_VERSION_MAJOR}::OpenGL Qt${QT_VERSION_MAJOR}::Concurrent Qt${QT_VERSION_MAJOR}::Multimedia
    Qwt::Qwt OpenGL::GL ZLIB::ZLIB OpenSSL::SSL OpenSSL::Crypto)
if(UNIX AND NOT APPLE)
    target_link_libraries(us_somo PUBLIC GLU)   # TODO: find_library(GLU) for robustness
endif()

# --- optional: ADOLC ---------------------------------------------------------
if(SOMO_ENABLE_ADOLC)
    find_path(ADOLC_INC NAMES adolc/adolc.h REQUIRED)
    find_library(ADOLC_LIB NAMES adolc REQUIRED)
    target_compile_definitions(us_somo PUBLIC ADOLC)
    target_include_directories(us_somo PUBLIC ${ADOLC_INC})
    target_link_libraries(us_somo PUBLIC ${ADOLC_LIB})
endif()

# --- optional: CUDA ----------------------------------------------------------
if(SOMO_ENABLE_CUDA)
    enable_language(CUDA)
    target_compile_definitions(us_somo PUBLIC CUDA)
    # us_saxs_util_cuda.cpp is already a lib member (it's in libus_somo.pro's
    # SOURCES); this option only turns on the CUDA code paths inside it.
    # TODO: wire the actual .cu kernels + CUDA::cudart.
endif()

# =============================================================================
# APPLICATIONS — two distinct classes (verified against the .pro files + ldd):
#
#   1) GUI apps  (include generic.pri): a single main.cpp that LINKS libus_somo
#      (which pulls qwt transitively).  -> us3_somo, us_admin, us3_config
#
#   2) Standalone command-line / MPI tools (include cmdline.pri): main.cpp plus
#      a CURATED subset of ../src/*.cpp compiled DIRECTLY into the executable.
#      They do NOT link libus_somo and do NOT link qwt (confirmed by ldd of the
#      deployed us_saxs_cmds_t).
# Each app's sources are read from its own .pro (which pulls main.cpp in from
# generic.pri / cmdline.pri), the same way as the library's.
# =============================================================================

# GUI app: main.cpp + link libus_somo (generic.pri: SOURCES=main.cpp, -lus_somo -lqwt)
function(somo_add_gui_app tgt pro)
    somo_qmake_sources(${SOMO_ROOT}/${pro} _srcs _hdrs)
    add_executable(${tgt} ${_srcs} ${_hdrs})
    target_link_libraries(${tgt} PRIVATE us_somo)
    target_include_directories(${tgt} PRIVATE ${SOMO_ROOT}/include ${SOMO_ROOT}/src)
    add_dependencies(${tgt} us_somo_genheaders)
endfunction()

# Standalone tool: its .pro's ../src subset, no libus_somo, no qwt (cmdline.pri).
function(somo_add_tool tgt)
    somo_qmake_sources(${SOMO_ROOT}/${tgt}/${tgt}.pro _srcs _hdrs)
    add_executable(${tgt} ${_srcs} ${_hdrs})
    # Tools compile SOMO headers that #include <qwt_*.h> (the qwt code paths are
    # compiled out under -DCMDLINE, so no qwt LINK is needed — matching the qmake
    # build/ldd — but the headers must be on the include path).
    target_include_directories(${tgt} PRIVATE
        ${SOMO_ROOT}/include ${SOMO_ROOT}/src ${SOMO_ROOT}/${tgt}
        ${SOMO_QWT_INCLUDE_DIR})
    target_compile_definitions(${tgt} PRIVATE CMDLINE NO_DB NO_EDITOR_PRINT)
    target_link_libraries(${tgt} PRIVATE
        Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Gui Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::Xml Qt${QT_VERSION_MAJOR}::Svg
        Qt${QT_VERSION_MAJOR}::PrintSupport Qt${QT_VERSION_MAJOR}::OpenGL Qt${QT_VERSION_MAJOR}::Concurrent Qt${QT_VERSION_MAJOR}::Multimedia
        OpenGL::GL ZLIB::ZLIB OpenSSL::SSL OpenSSL::Crypto)
    if(UNIX AND NOT APPLE)
        target_compile_definitions(${tgt} PRIVATE UNIX LINUX BIN64)
        target_link_libraries(${tgt} PRIVATE GLU)
    endif()
    add_dependencies(${tgt} us_somo_genheaders)
endfunction()

# --- GUI applications --------------------------------------------------------
somo_add_gui_app(us3_somo   us3_hydrodyn/us3_hydrodyn.pro)   # main GUI -> us3_somo binary
somo_add_gui_app(us_admin   us_admin/us_admin.pro)
somo_add_gui_app(us3_config us3_config/us3_config.pro)

# --- standalone tools --------------------------------------------------------
somo_add_tool(us_saxs_cmds_t)                 # non-MPI SAXS command-line tool
# us_cluster_server_t (2011) dropped: its us_cluster_server_global.h includes
# <q3socket.h> (Qt3Support), removed since Qt5 — it cannot build on Qt5/Qt6.

set(SOMO_INSTALL_TARGETS
    us_somo us3_somo us_admin us3_config us_saxs_cmds_t)

# --- optional MPI tools ------------------------------------------------------
if(SOMO_ENABLE_MPI)
    find_package(MPI REQUIRED CXX)

    # USE_MPI is the code's MPI guard (DEFINES += USE_MPI in both MPI .pro files)
    somo_add_tool(us_saxs_cmds_mpi)
    target_link_libraries(us_saxs_cmds_mpi PRIVATE MPI::MPI_CXX)
    target_compile_definitions(us_saxs_cmds_mpi PRIVATE USE_MPI OMPI_SKIP_MPICXX)
    target_include_directories(us_saxs_cmds_mpi PRIVATE ${MPI_CXX_INCLUDE_DIRS})
    list(APPEND SOMO_INSTALL_TARGETS us_saxs_cmds_mpi)

    if(SOMO_ENABLE_CUDA)
        somo_add_tool(us_saxs_cmds_mpi_cuda)
        target_link_libraries(us_saxs_cmds_mpi_cuda PRIVATE MPI::MPI_CXX)
        target_compile_definitions(us_saxs_cmds_mpi_cuda PRIVATE USE_MPI OMPI_SKIP_MPICXX CUDA)
        target_include_directories(us_saxs_cmds_mpi_cuda PRIVATE ${MPI_CXX_INCLUDE_DIRS})
        list(APPEND SOMO_INSTALL_TARGETS us_saxs_cmds_mpi_cuda)
    endif()
endif()

# --- install -----------------------------------------------------------------
install(TARGETS ${SOMO_INSTALL_TARGETS}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib)

message(STATUS "SOMO: standalone=${SOMO_STANDALONE} "
               "MPI=${SOMO_ENABLE_MPI} CUDA=${SOMO_ENABLE_CUDA} ADOLC=${SOMO_ENABLE_ADOLC}")
