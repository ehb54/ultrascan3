# UsDefinesParser.cmake
# Extracts version and description from us_defines.h

function(parse_header HEADER_FILE)
    if(NOT EXISTS "${HEADER_FILE}")
        message(FATAL_ERROR "Header file not found: ${HEADER_FILE}")
    endif()

    # Read the header file
    file(READ "${HEADER_FILE}" HEADER_CONTENT)

    # Extract the version string from US_Version define
    # Looking for: #define US_Version QString("X.Y.Z-dev")
    if(HEADER_CONTENT MATCHES "#define[ \t]+US_Version[ \t]+QString\\(\"([0-9]+)\\.([0-9]+)\\.([0-9]+)(-[a-zA-Z0-9.+]+)?\"\\)")
        set(MAJOR "${CMAKE_MATCH_1}")
        set(MINOR "${CMAKE_MATCH_2}")
        set(PATCH "${CMAKE_MATCH_3}")
        set(SUFFIX "${CMAKE_MATCH_4}")

        set(US3_VERSION_MAJOR "${MAJOR}" PARENT_SCOPE)
        set(US3_VERSION_MINOR "${MINOR}" PARENT_SCOPE)
        set(US3_VERSION_PATCH "${PATCH}" PARENT_SCOPE)
        set(US3_VERSION "${MAJOR}.${MINOR}.${PATCH}${SUFFIX}" PARENT_SCOPE)

        message(STATUS "Extracted version: ${MAJOR}.${MINOR}.${PATCH}${SUFFIX}")
        message(STATUS "  Major: ${MAJOR}")
        message(STATUS "  Minor: ${MINOR}")
        message(STATUS "  Patch: ${PATCH}")
    else()
        message(FATAL_ERROR "Could not extract US_Version from ${HEADER_FILE}")
    endif()

    # Extract the description string (optional)
    # Looking for: #define US_Description "text"
    if(HEADER_CONTENT MATCHES "#define[ \t]+US_Description[ \t]+\"([^\"]+)\"")
        set(US3_DESCRIPTION "${CMAKE_MATCH_1}" PARENT_SCOPE)
        message(STATUS "Extracted description: ${CMAKE_MATCH_1}")
    else()
        # Use default if not found
        set(US3_DESCRIPTION "UltraScan3 - Advanced analysis of analytical ultracentrifugation experiments" PARENT_SCOPE)
        message(STATUS "No US_Description found, using default")
    endif()
endfunction()