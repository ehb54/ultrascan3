# admin/cmake/Us3Resources.cmake
include(CMakeParseArguments)

# us3_generate_qrc_from_dir(
#   OUT_QRC <file>
#   PREFIX  <:/prefix>
#   BASE    <base_dir>
#   GLOBS   <patterns...>        # e.g. "*.png;*.ico;*.xpm;*.svg;*.xml"
#   EXCLUDE_DIRS <dir...>        # relative to BASE, e.g. "rotors" "dmd"
#   EXCLUDE_FILES <file...>      # relative to BASE, e.g. "cluster.config"
#   USE_ABSOLUTE                 # Optional: use absolute paths with aliases
# )
function(us3_generate_qrc_from_dir)
    cmake_parse_arguments(US3 "USE_ABSOLUTE" "OUT_QRC;PREFIX;BASE"
            "GLOBS;EXCLUDE_DIRS;EXCLUDE_FILES" ${ARGN})
    if(NOT US3_OUT_QRC OR NOT US3_PREFIX OR NOT US3_BASE)
        message(FATAL_ERROR "us3_generate_qrc_from_dir: OUT_QRC, PREFIX, BASE are required")
    endif()

    # Collect all files
    set(ALL_FILES)
    if(US3_GLOBS)
        foreach(p IN LISTS US3_GLOBS)
            file(GLOB_RECURSE _tmp CONFIGURE_DEPENDS "${US3_BASE}/${p}")
            list(APPEND ALL_FILES ${_tmp})
        endforeach()
    else()
        file(GLOB_RECURSE ALL_FILES CONFIGURE_DEPENDS "${US3_BASE}/*.*")
    endif()

    # Normalize BASE path
    file(TO_CMAKE_PATH "${US3_BASE}" _BASE)

    # Filter files and build lists
    set(FINAL_REL)
    set(FINAL_ABS)
    foreach(abs IN LISTS ALL_FILES)
        file(RELATIVE_PATH rel "${_BASE}" "${abs}")
        set(skip FALSE)

        # Check excluded directories
        foreach(exd IN LISTS US3_EXCLUDE_DIRS)
            if(rel MATCHES "^${exd}(/|$)")
                set(skip TRUE)
                break()
            endif()
        endforeach()

        # Check excluded files
        foreach(exf IN LISTS US3_EXCLUDE_FILES)
            if(rel STREQUAL "${exf}")
                set(skip TRUE)
                break()
            endif()
        endforeach()

        if(NOT skip)
            list(APPEND FINAL_REL "${rel}")
            list(APPEND FINAL_ABS "${abs}")
        endif()
    endforeach()

    # Sort by relative path for stable output
    set(TUP)
    list(LENGTH FINAL_REL len)
    if(len GREATER 0)
        math(EXPR last "${len} - 1")
        foreach(i RANGE 0 ${last})
            list(GET FINAL_REL ${i} r)
            list(GET FINAL_ABS ${i} a)
            list(APPEND TUP "${r}|${a}")
        endforeach()
    endif()
    list(SORT TUP)

    # Rebuild sorted lists
    set(FINAL_REL)
    set(FINAL_ABS)
    foreach(t IN LISTS TUP)
        string(REPLACE "|" ";" pair "${t}")
        list(GET pair 0 rel_part)
        list(GET pair 1 abs_part)
        list(APPEND FINAL_REL "${rel_part}")
        list(APPEND FINAL_ABS "${abs_part}")
    endforeach()

    # Write QRC file
    file(WRITE  "${US3_OUT_QRC}" "<RCC>\n  <qresource prefix=\"${US3_PREFIX}\">\n")
    list(LENGTH FINAL_REL len)
    if(len GREATER 0)
        math(EXPR last "${len} - 1")
        foreach(i RANGE 0 ${last})
            list(GET FINAL_REL ${i} rel)
            if(US3_USE_ABSOLUTE)
                list(GET FINAL_ABS ${i} abs)
                file(APPEND "${US3_OUT_QRC}" "    <file alias=\"${rel}\">${abs}</file>\n")
            else()
                file(APPEND "${US3_OUT_QRC}" "    <file>${rel}</file>\n")
            endif()
        endforeach()
    endif()
    file(APPEND "${US3_OUT_QRC}" "  </qresource>\n</RCC>\n")

    message(STATUS "Generated QRC: ${US3_OUT_QRC} (${len} files from ${_BASE})")
endfunction()