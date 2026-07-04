# =============================================================================
# us_somo / US-SOMO  —  CMake build  (SKELETON — not yet buildable end-to-end)
# =============================================================================
# Builds the SOMO hydrodynamics / SAXS suite. Two classes of application:
#   GUI apps — a single main.cpp that LINKS libus_somo (+ qwt):
#     libus_somo          shared library (SONAME .10) — the bulk of the code
#     us3_somo            main GUI application     (us3_hydrodyn/main.cpp)
#     us_admin            admin GUI                (us_admin/)
#     us3_config          config GUI               (us3_config/)
#   Standalone tools — compile a curated ../src subset directly, NO lib, NO qwt:
#     us_saxs_cmds_t      SAXS command-line tool   (us_saxs_cmds_t/)
#     us_cluster_server_t legacy cluster-server test tool (2011; may be obsolete)
# Optional (default OFF):
#     us_saxs_cmds_mpi        MPI SAXS tool            SOMO_ENABLE_MPI
#     us_saxs_cmds_mpi_cuda   MPI+CUDA SAXS tool       SOMO_ENABLE_MPI + SOMO_ENABLE_CUDA
#     CUDA code paths in the lib                       SOMO_ENABLE_CUDA
#     ADOLC autodiff                                   SOMO_ENABLE_ADOLC
#
# KEY FACT (verified): SOMO is SELF-CONTAINED. It VENDORS its own fork of the
# us_utils sources (src/us_math.cpp, us_matrix.cpp, us_saxs_util*.cpp, ...) and
# does NOT link UltraScan3::Utils. Its only couplings are third-party: Qt5,
# Qwt, QwtPlot3D, OpenGL, zlib, OpenSSL. Runtime ldd of the deployed us3_somo
# confirms: libus_somo.so.10 + qwt + Qt + system libs, no libus_utils.so.
#
# DUAL-MODE: this file configures standalone
#     cmake -S us_somo/develop -B build --toolchain <vcpkg>/scripts/buildsystems/vcpkg.cmake
# OR when pulled in from the root UltraScan3 build via add_subdirectory(us_somo).
#
# In standalone mode the third-party deps come from this directory's own
# vcpkg.json (+ vcpkg-configuration.json), a minimal manifest that reuses the
# repo-root overlay-ports (qwt / qwtplot3d / qt5-multimedia) and the same
# builtin-baseline as the root. It has qt5 (default) and qt6 features — select
# Qt6 with: --x-feature=qt6 (or -DVCPKG_MANIFEST_FEATURES=qt6 -DVCPKG_MANIFEST_NO_DEFAULT_FEATURES=ON).
# When built from the root instead, the root vcpkg.json (qt5-* / qt6-app
# features) already provides every dep SOMO needs, and this manifest is simply
# ignored (vcpkg reads the manifest at the top-level source).
# =============================================================================

# --- standalone bootstrap (skipped when included via add_subdirectory) --------
if(NOT DEFINED PROJECT_NAME)
    cmake_minimum_required(VERSION 3.21)
    project(us_somo VERSION 10.0.0 LANGUAGES CXX C)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    set(SOMO_STANDALONE ON)
else()
    set(SOMO_STANDALONE OFF)   # inherit CXX standard / output dirs from root
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

# --- dependencies (all third-party) ------------------------------------------
# Qt5 AND Qt6: PR #335 made the SOMO code compatible with both; CI/CodeQL test
# both, and the qmake build is Qt5 today with a planned move to Qt6. Use the
# version-agnostic idiom so this follows whichever Qt the (GUI) build selects,
# and link Qt${QT_VERSION_MAJOR}::... throughout.
# NOTE: Multimedia is required (sounds/). The root manifest's qt5-app/qt6-app
# features include qt5-multimedia / qtmultimedia, and the standalone vcpkg.json
# provides both via its qt5 / qt6 features.
# TODO(Qt6): QtOpenGL split in Qt6 — if the code uses QOpenGLWidget, add the
# OpenGLWidgets component for QT_VERSION_MAJOR==6.
set(SOMO_QT_COMPONENTS Core Gui Widgets Network Xml Svg PrintSupport OpenGL Concurrent Multimedia)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
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

# QwtPlot3D — SOMO carries its own 3dplot/ tree.
# TODO: decide between (a) reusing root qwtplot3d::qwtplot3d, or
#       (b) add_subdirectory(3dplot) for the SOMO-local copy.
if(NOT TARGET qwtplot3d::qwtplot3d)
    # add_subdirectory(3dplot)   # TODO: provide 3dplot/CMakeLists.txt
    message(WARNING "SOMO: no qwtplot3d::qwtplot3d target yet (3dplot TODO)")
endif()

# --- generated version headers -----------------------------------------------
# SOMO generates two headers via shell scripts (as in the qmake build):
#   version.sh  -> include/us_version.h   (needs $us3      = repo root; reads utils/us_defines.h)
#   revision.sh -> include/us_revision.h  (needs $ULTRASCAN = repo root; git log)
# us_config.cpp includes us_version.h, and many src/ files include us_revision.h,
# so both must exist before compilation. Both need git.
# TODO: these write into the SOURCE tree and shell out to git — prefer a
# CMake-native generator writing into ${CMAKE_CURRENT_BINARY_DIR} (see
# admin/cmake/GenerateVersion.cmake). Generated headers are gitignored.
get_filename_component(SOMO_REPO_ROOT "${SOMO_ROOT}/../.." ABSOLUTE)
add_custom_target(us_somo_genheaders ALL
    COMMAND ${CMAKE_COMMAND} -E env "us3=${SOMO_REPO_ROOT}" "ULTRASCAN=${SOMO_REPO_ROOT}" sh version.sh
    COMMAND ${CMAKE_COMMAND} -E env "us3=${SOMO_REPO_ROOT}" "ULTRASCAN=${SOMO_REPO_ROOT}" sh revision.sh
    WORKING_DIRECTORY ${SOMO_ROOT}
    BYPRODUCTS ${SOMO_ROOT}/include/us_version.h ${SOMO_ROOT}/include/us_revision.h
    COMMENT "Generating SOMO us_version.h / us_revision.h")

# --- the library: libus_somo -------------------------------------------------
# AUTHORITATIVE source/header lists, extracted from libus_somo.pro (SOURCES /
# HEADERS) into somo_lib_sources.cmake — NOT a glob of src/*.cpp. Globbing is
# wrong here: several src/ TUs are app-specific and must not be in the lib, and
# the lib also pulls in shd_mpi/shs_use.cpp. Regenerate the .cmake from the .pro
# if the source set changes. Apps are thin main() wrappers that link this lib.
include(${SOMO_ROOT}/somo_lib_sources.cmake)   # -> SOMO_LIB_SOURCES / SOMO_LIB_HEADERS (paths relative to develop/)
list(TRANSFORM SOMO_LIB_SOURCES PREPEND "${SOMO_ROOT}/")
list(TRANSFORM SOMO_LIB_HEADERS PREPEND "${SOMO_ROOT}/")

add_library(us_somo SHARED ${SOMO_LIB_SOURCES} ${SOMO_LIB_HEADERS})
add_dependencies(us_somo us_somo_genheaders)
set_target_properties(us_somo PROPERTIES VERSION 10.0.0 SOVERSION 10)  # libus_somo.so.10

target_include_directories(us_somo PUBLIC ${SOMO_ROOT}/include ${SOMO_ROOT}/src)

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
if(TARGET qwtplot3d::qwtplot3d)
    target_link_libraries(us_somo PUBLIC qwtplot3d::qwtplot3d)
endif()
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
#      deployed us_saxs_cmds_t). Their exact source subsets live in
#      somo_app_sources.cmake, extracted from each tool's own .pro.
# =============================================================================
include(${SOMO_ROOT}/somo_app_sources.cmake)   # -> SOMO_TOOL_<name>_SOURCES / _HEADERS

# GUI app: main.cpp + link libus_somo (generic.pri: SOURCES=main.cpp, -lus_somo -lqwt)
function(somo_add_gui_app tgt subdir)
    add_executable(${tgt} ${SOMO_ROOT}/${subdir}/main.cpp)
    target_link_libraries(${tgt} PRIVATE us_somo)
    target_include_directories(${tgt} PRIVATE ${SOMO_ROOT}/include ${SOMO_ROOT}/src)
    add_dependencies(${tgt} us_somo_genheaders)
endfunction()

# Standalone tool: explicit ../src subset, no libus_somo, no qwt (cmdline.pri).
function(somo_add_tool tgt)
    set(_srcs ${SOMO_TOOL_${tgt}_SOURCES})
    set(_hdrs ${SOMO_TOOL_${tgt}_HEADERS})
    list(TRANSFORM _srcs PREPEND "${SOMO_ROOT}/")
    list(TRANSFORM _hdrs PREPEND "${SOMO_ROOT}/")
    add_executable(${tgt} ${_srcs} ${_hdrs})
    # Tools compile SOMO headers that #include <qwt_*.h> (the qwt code paths are
    # compiled out under -DCMDLINE, so no qwt LINK is needed — matching the qmake
    # build/ldd — but the headers must be on the include path).
    target_include_directories(${tgt} PRIVATE
        ${SOMO_ROOT}/include ${SOMO_ROOT}/src ${SOMO_ROOT}/${tgt}
        $<TARGET_PROPERTY:Qwt::Qwt,INTERFACE_INCLUDE_DIRECTORIES>)
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
somo_add_gui_app(us3_somo   us3_hydrodyn)     # main GUI -> us3_somo binary
somo_add_gui_app(us_admin   us_admin)
somo_add_gui_app(us3_config us3_config)

# --- standalone tools --------------------------------------------------------
somo_add_tool(us_saxs_cmds_t)                 # non-MPI SAXS command-line tool
somo_add_tool(us_cluster_server_t)            # legacy (2011) cluster-server test tool; may be obsolete

set(SOMO_INSTALL_TARGETS
    us_somo us3_somo us_admin us3_config us_saxs_cmds_t us_cluster_server_t)

# --- optional MPI tools ------------------------------------------------------
if(SOMO_ENABLE_MPI)
    find_package(MPI REQUIRED CXX)

    somo_add_tool(us_saxs_cmds_mpi)
    target_link_libraries(us_saxs_cmds_mpi PRIVATE MPI::MPI_CXX)
    # TODO: MPI flags come from a custom qmake mkspec in the qmake build, not the
    # .pro — confirm whether a project-specific MPI guard define is also needed.
    target_compile_definitions(us_saxs_cmds_mpi PRIVATE OMPI_SKIP_MPICXX)
    target_include_directories(us_saxs_cmds_mpi PRIVATE ${MPI_CXX_INCLUDE_DIRS})
    list(APPEND SOMO_INSTALL_TARGETS us_saxs_cmds_mpi)

    if(SOMO_ENABLE_CUDA)
        somo_add_tool(us_saxs_cmds_mpi_cuda)
        target_link_libraries(us_saxs_cmds_mpi_cuda PRIVATE MPI::MPI_CXX)
        target_compile_definitions(us_saxs_cmds_mpi_cuda PRIVATE OMPI_SKIP_MPICXX CUDA)
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
