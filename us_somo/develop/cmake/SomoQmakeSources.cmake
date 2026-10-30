# =============================================================================
# SomoQmakeSources.cmake — read SOURCES / HEADERS from SOMO's qmake files
# =============================================================================
# While qmake and CMake coexist, the .pro files stay the ONE list of what gets
# compiled. Every branch (main, somo-dev, issue branches) carries its own .pro
# lists, so reading them at configure time lets the same CMake files build any
# branch with no second list to keep in step. When qmake is retired, write the
# lists out as plain CMake one last time and drop this module.
#
#   somo_qmake_sources(<pro-file> <sources-var> <headers-var> [EXCLUDE <regex>...])
#
# Returns absolute paths. EXCLUDE drops entries matching any of the regexes
# (e.g. generated headers that a .pro lists but CMake produces itself).
#
# The qmake subset understood here is everything SOMO's .pro/.pri files use:
#   - SOURCES / HEADERS with  =  (reset),  +=  and  *=  (append),  -=  (remove)
#   - backslash continuations; a whole-line # comment inside a continued list
#     is skipped and the list carries on (as qmake does); trailing # comments
#   - include( file.pri ), resolved against the including file's directory;
#     the untracked, per-developer local.pri and missing files are skipped
#   - relative entries resolve against the top-level .pro's directory (qmake's
#     $$_PRO_FILE_PWD_), even when they come from an included .pri
# Anything else that touches SOURCES/HEADERS — inside a { } scope, behind a
# "unix:" condition, using a $$variable — stops the configure step and names
# the file, instead of being silently misread. Every file read is added to
# CMAKE_CONFIGURE_DEPENDS, so editing a .pro re-runs the configure step.
# =============================================================================
include_guard(GLOBAL)

# Reads one .pro/.pri and updates _somo_qm_SOURCES / _somo_qm_HEADERS in the
# caller's scope (include() recurses, and each level hands the lists back up
# with PARENT_SCOPE).
function(_somo_qmake_read file pro_dir)
    file(READ "${file}" text)
    string(REPLACE "\r" "" text "${text}")
    # whole-line comments go with their newline, so a continued list runs on
    string(REGEX REPLACE "\n[ \t]*#[^\n]*" "" text "\n${text}")
    string(REGEX REPLACE "#[^\n]*" "" text "${text}")
    string(REGEX REPLACE "\\\\[ \t]*\n" " " text "${text}")
    # CMake list syntax; none of these can appear in a SOMO file name
    string(REPLACE ";" " " text "${text}")
    string(REPLACE "[" " " text "${text}")
    string(REPLACE "]" " " text "${text}")
    string(REPLACE "\\" "/" text "${text}")
    string(REPLACE "\n" ";" lines "${text}")

    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${file}")
    get_filename_component(file_dir "${file}" DIRECTORY)
    set(depth 0)

    foreach(line IN LISTS lines)
        if(line MATCHES "^[ \t]*include[ \t]*\\(([^)]*)\\)[ \t]*$")
            string(STRIP "${CMAKE_MATCH_1}" inc)
            cmake_path(ABSOLUTE_PATH inc BASE_DIRECTORY "${file_dir}" NORMALIZE)
            get_filename_component(inc_name "${inc}" NAME)
            if(NOT inc_name STREQUAL "local.pri" AND EXISTS "${inc}")
                if(depth GREATER 0)
                    message(FATAL_ERROR "SOMO: conditional include() is not supported "
                                        "(${file}): ${line}")
                endif()
                _somo_qmake_read("${inc}" "${pro_dir}")
            endif()
        elseif(line MATCHES "^[ \t]*(SOURCES|HEADERS)[ \t]*([-+*]?=)(.*)$")
            set(var "${CMAKE_MATCH_1}")
            set(op "${CMAKE_MATCH_2}")
            set(rest "${CMAKE_MATCH_3}")
            if(depth GREATER 0)
                message(FATAL_ERROR "SOMO: ${var} inside a qmake scope is not supported "
                                    "(${file}): ${line}")
            endif()
            if(rest MATCHES "\\$")
                message(FATAL_ERROR "SOMO: qmake variables in ${var} are not supported "
                                    "(${file}): ${line}")
            endif()
            string(REGEX MATCHALL "[^ \t]+" items "${rest}")
            set(paths "")
            foreach(item IN LISTS items)
                cmake_path(ABSOLUTE_PATH item BASE_DIRECTORY "${pro_dir}" NORMALIZE)
                list(APPEND paths "${item}")
            endforeach()
            if(op STREQUAL "=")
                set(_somo_qm_${var} ${paths})
            elseif(op STREQUAL "-=")
                if(paths)
                    list(REMOVE_ITEM _somo_qm_${var} ${paths})
                endif()
            else()
                list(APPEND _somo_qm_${var} ${paths})
            endif()
        elseif(line MATCHES "(^|[^A-Za-z0-9_])(SOURCES|HEADERS)([^A-Za-z0-9_]|$)")
            message(FATAL_ERROR "SOMO: unsupported qmake construct for "
                                "${CMAKE_MATCH_2} (${file}): ${line}")
        endif()

        # { } scope depth, for the lines that follow
        string(REGEX MATCHALL "{" opens "${line}")
        string(REGEX MATCHALL "}" closes "${line}")
        list(LENGTH opens n_open)
        list(LENGTH closes n_close)
        math(EXPR depth "${depth} + ${n_open} - ${n_close}")
    endforeach()

    set(_somo_qm_SOURCES "${_somo_qm_SOURCES}" PARENT_SCOPE)
    set(_somo_qm_HEADERS "${_somo_qm_HEADERS}" PARENT_SCOPE)
endfunction()

function(somo_qmake_sources pro sources_var headers_var)
    cmake_parse_arguments(PARSE_ARGV 3 arg "" "" "EXCLUDE")
    cmake_path(ABSOLUTE_PATH pro BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" NORMALIZE)
    if(NOT EXISTS "${pro}")
        message(FATAL_ERROR "SOMO: qmake project ${pro} not found")
    endif()
    get_filename_component(pro_dir "${pro}" DIRECTORY)

    set(_somo_qm_SOURCES "")
    set(_somo_qm_HEADERS "")
    _somo_qmake_read("${pro}" "${pro_dir}")

    foreach(var SOURCES HEADERS)
        list(REMOVE_DUPLICATES _somo_qm_${var})
        foreach(re IN LISTS arg_EXCLUDE)
            list(FILTER _somo_qm_${var} EXCLUDE REGEX "${re}")
        endforeach()
        set(missing "")
        foreach(f IN LISTS _somo_qm_${var})
            if(NOT EXISTS "${f}")
                list(APPEND missing "${f}")
            endif()
        endforeach()
        if(missing)
            list(JOIN missing "\n  " missing)
            message(FATAL_ERROR "SOMO: ${pro} lists ${var} that do not exist:\n  ${missing}")
        endif()
    endforeach()

    set(${sources_var} "${_somo_qm_SOURCES}" PARENT_SCOPE)
    set(${headers_var} "${_somo_qm_HEADERS}" PARENT_SCOPE)
endfunction()
