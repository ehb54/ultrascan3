# =============================================================================
# us_somo / US-SOMO  —  CMake build  (SKELETON — not yet buildable end-to-end)
# =============================================================================
# Builds the SOMO hydrodynamics / SAXS suite:
#     libus_somo        shared library (SONAME .10) — the bulk of the code
#     us3_somo          main GUI application     (us3_hydrodyn/main.cpp)
#     us_admin          admin GUI                (us_admin/)
#     us3_config        config GUI               (us3_config/)
#     us_saxs_cmds_t    headless SAXS tool       (us_saxs_cmds_t/)
# Optional (default OFF):
#     us_saxs_cmds_mpi  MPI SAXS tool            SOMO_ENABLE_MPI
#     CUDA SAXS kernels                          SOMO_ENABLE_CUDA
#     ADOLC autodiff                             SOMO_ENABLE_ADOLC
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
# NOTE: Multimedia is required (sounds/) and is NOT in the root's HPC Qt set —
# the root vcpkg manifest's qt5-app / qt6-app features already include
# qt5-multimedia / qtmultimedia, so an integrated build is covered.
find_package(Qt5 REQUIRED COMPONENTS
    Core Gui Widgets Network Xml Svg PrintSupport OpenGL Concurrent Multimedia)
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

# --- revision header (mirrors qmake `revision.sh` -> include/us_revision.h) ---
# TODO: revision.sh currently writes into the SOURCE tree. Prefer generating
# into ${CMAKE_CURRENT_BINARY_DIR} and adding that to the include path, to keep
# the checkout clean (see admin/cmake/GenerateVersion.cmake for the pattern).
add_custom_target(us_somo_revision ALL
    COMMAND sh ${SOMO_ROOT}/revision.sh
    WORKING_DIRECTORY ${SOMO_ROOT}
    BYPRODUCTS ${SOMO_ROOT}/include/us_revision.h
    COMMENT "Generating SOMO us_revision.h")

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
add_dependencies(us_somo us_somo_revision)
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
    Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Network Qt5::Xml Qt5::Svg
    Qt5::PrintSupport Qt5::OpenGL Qt5::Concurrent Qt5::Multimedia
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

# --- helper: declare a thin SOMO app that links libus_somo -------------------
function(somo_add_app tgt subdir)
    cmake_parse_arguments(A "" "" "EXTRA_SOURCES" ${ARGN})
    file(GLOB _srcs CONFIGURE_DEPENDS
        ${SOMO_ROOT}/${subdir}/*.cpp
        ${SOMO_ROOT}/${subdir}/*.ui
        ${SOMO_ROOT}/${subdir}/*.qrc)
    add_executable(${tgt} ${_srcs} ${A_EXTRA_SOURCES})
    target_link_libraries(${tgt} PRIVATE us_somo)
    target_include_directories(${tgt} PRIVATE ${SOMO_ROOT}/include ${SOMO_ROOT}/src)
    add_dependencies(${tgt} us_somo_revision)
endfunction()

# --- applications ------------------------------------------------------------
# TODO: each app dir is currently a single main.cpp (verified), so a glob is
# adequate for now — but for parity with the lib, extract each app's SOURCES
# from its own .pro (us3_hydrodyn.pro, us_admin.pro, ...) before this is final.
somo_add_app(us3_somo       us3_hydrodyn)     # main GUI -> us3_somo binary
somo_add_app(us_admin       us_admin)
somo_add_app(us3_config     us3_config)
somo_add_app(us_saxs_cmds_t us_saxs_cmds_t)   # cuda TU comes from the lib; do not re-add here

if(SOMO_ENABLE_MPI)
    find_package(MPI REQUIRED CXX)
    somo_add_app(us_saxs_cmds_mpi us_saxs_cmds_mpi)
    target_link_libraries(us_saxs_cmds_mpi PRIVATE MPI::MPI_CXX)
    target_compile_definitions(us_saxs_cmds_mpi PRIVATE OMPI_SKIP_MPICXX)
    target_include_directories(us_saxs_cmds_mpi PRIVATE ${MPI_CXX_INCLUDE_DIRS})
endif()

# --- install -----------------------------------------------------------------
install(TARGETS us_somo us3_somo us_admin us3_config us_saxs_cmds_t
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib)

message(STATUS "SOMO: standalone=${SOMO_STANDALONE} "
               "MPI=${SOMO_ENABLE_MPI} CUDA=${SOMO_ENABLE_CUDA} ADOLC=${SOMO_ENABLE_ADOLC}")
