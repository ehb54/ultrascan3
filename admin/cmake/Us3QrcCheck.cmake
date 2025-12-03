# Args: -DUS3_QRC=<path to generated qrc> -DUS3_BASE=<etc dir>
if(NOT DEFINED US3_QRC OR NOT DEFINED US3_BASE)
    message(FATAL_ERROR "Us3QrcCheck.cmake: US3_QRC and US3_BASE required")
endif()

if(NOT EXISTS "${US3_QRC}")
    message(FATAL_ERROR "QRC file not found: ${US3_QRC}")
endif()

file(READ "${US3_QRC}" QRC_TXT)
string(REGEX MATCHALL "<file[^>]*>[^<]+</file>" FILE_TAGS "${QRC_TXT}")
set(MISSING "")
set(CHECKED 0)

foreach(tag IN LISTS FILE_TAGS)
    # Extract file path (handles both <file>path</file> and <file alias="...">path</file>)
    string(REGEX REPLACE "^<file[^>]*>|</file>$" "" FILE_PATH "${tag}")

    # Check if path is absolute or relative
    if(IS_ABSOLUTE "${FILE_PATH}")
        set(FULL_PATH "${FILE_PATH}")
    else()
        set(FULL_PATH "${US3_BASE}/${FILE_PATH}")
    endif()

    if(NOT EXISTS "${FULL_PATH}")
        list(APPEND MISSING "${FILE_PATH}")
    endif()
    math(EXPR CHECKED "${CHECKED} + 1")
endforeach()

list(LENGTH MISSING MISS_N)
if(MISS_N GREATER 0)
    message(FATAL_ERROR
            "QRC references ${MISS_N} missing file(s) out of ${CHECKED} checked:\n  ${MISSING}\n"
            "Tip: restore files, or add them to EXCLUDE_FILES, then reconfigure.")
else()
    message(STATUS "QRC validation passed: ${CHECKED} files verified in ${US3_QRC}")
endif()
