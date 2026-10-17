# SomoGenHeaders.cmake — writes SOMO's version headers, as version.sh and
# revision.sh do for the qmake build, with nothing but CMake and git: no sh,
# so it also runs where there is no POSIX shell (MSVC, plain Windows).
#
# Run at build time by the us_somo_genheaders target:
#   cmake -DSOMO_ROOT=<us_somo/develop> -DREPO_ROOT=<repository root>
#         -DGIT_EXECUTABLE=<git, may be empty>
#         [-DSOMO_GEN_VERSION=ON] [-DSOMO_GEN_REVISION=ON]
#         -P SomoGenHeaders.cmake
#
#   include/us_version.h   US_Version from utils/us_defines.h, and SOMO's
#                          revision: the number of commits touching us_somo/
#   include/us_revision.h  the date of the last commit touching us_somo/, and
#                          its position in the history (git log order)
#
# A header is rewritten only when its contents change, so an unchanged
# revision does not recompile the files that include it.
#
# us_somo/develop also builds on its own, without the rest of the repository
# or without git (a copy of the directory). Then a header that already exists
# is kept as it is (somo-dev commits both); a missing one is written with
# version "unknown" and revision 0, as the scripts give without git.

foreach(_var SOMO_ROOT REPO_ROOT)
    if(NOT ${_var})
        message(FATAL_ERROR "SomoGenHeaders.cmake: ${_var} is not set")
    endif()
endforeach()

# git output, or "" if git is missing or fails (not a repository, ...). It runs
# in us_somo/develop, which always exists, so the pathspecs below name
# us_somo/ as "..".
function(_somo_git out)
    set(_res "")
    if(GIT_EXECUTABLE)
        execute_process(COMMAND "${GIT_EXECUTABLE}" ${ARGN}
                        WORKING_DIRECTORY "${SOMO_ROOT}"
                        OUTPUT_VARIABLE _res RESULT_VARIABLE _rc
                        OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
        if(NOT _rc EQUAL 0)
            set(_res "")
        endif()
    endif()
    set(${out} "${_res}" PARENT_SCOPE)
endfunction()

# write <file> only if its contents differ
function(_somo_write file contents)
    if(EXISTS "${file}")
        file(READ "${file}" _old)
        if(_old STREQUAL contents)
            message(STATUS "SOMO: ${file} unchanged")
            return()
        endif()
    endif()
    file(WRITE "${file}" "${contents}")
    message(STATUS "SOMO: wrote ${file}")
endfunction()

if(SOMO_GEN_VERSION)
    set(_hdr "${SOMO_ROOT}/include/us_version.h")
    set(_ver "")
    set(_defines "${REPO_ROOT}/utils/us_defines.h")
    if(EXISTS "${_defines}")
        file(STRINGS "${_defines}" _lines REGEX "^#define[ \t]+US_Version[ \t]")
    else()
        # a sparse checkout may have it in git but not on disk
        _somo_git(_text show HEAD:utils/us_defines.h)
        string(REGEX MATCHALL "#define[ \t]+US_Version[ \t][^\n]*" _lines "${_text}")
    endif()
    if(_lines)
        list(GET _lines 0 _line)
        string(REGEX REPLACE "^[^\"]*\"([^\"]*)\".*$" "\\1" _ver "${_line}")
    endif()

    if(_ver STREQUAL "" AND EXISTS "${_hdr}")
        message(STATUS "SOMO: no utils/us_defines.h here (only us_somo/develop?); keeping ${_hdr}")
    else()
        if(_ver STREQUAL "")
            message(WARNING "SOMO: no utils/us_defines.h here (only us_somo/develop?); "
                            "writing ${_hdr} with version \"unknown\"")
            set(_ver unknown)
        endif()
        # version.sh: git log --oneline . | wc -l, in us_somo/
        _somo_git(_count rev-list --count HEAD -- ..)
        if(_count STREQUAL "")
            set(_count 0)
        endif()
        _somo_write("${_hdr}"
"#ifndef US_VERSION_H
#define US_VERSION_H

#define US_Version_string \"${_ver}\"
#define WIN32Version      \"-WIN32-${_ver}\"
#define SOMO_Revision     \"SOMOgit-${_count}\"

#endif
")
    endif()
endif()

if(SOMO_GEN_REVISION)
    set(_hdr "${SOMO_ROOT}/include/us_revision.h")
    # revision.sh: the date of the last commit touching us_somo/, and the
    # number of commits from it to the end of `git log` (it included)
    _somo_git(_hash log -1 --format=%H -- ..)
    if(_hash STREQUAL "" AND EXISTS "${_hdr}")
        message(STATUS "SOMO: no git history here; keeping ${_hdr}")
    else()
        set(_date "")
        set(_number 0)
        if(NOT _hash STREQUAL "")
            _somo_git(_date log -1 --format=%ad -- ..)
            _somo_git(_all log --format=%H)
            string(REPLACE "\n" ";" _all "${_all}")
            list(LENGTH _all _n)
            list(FIND _all "${_hash}" _pos)
            if(_pos GREATER_EQUAL 0)
                math(EXPR _number "${_n} - ${_pos}")
            endif()
        else()
            message(STATUS "SOMO: no git history here; writing ${_hdr} with revision 0")
        endif()
        _somo_write("${_hdr}"
"#define REVISION \"Revision: ${_number}\"
#define REVISION_DATE \"${_date}\"
")
    endif()
endif()
