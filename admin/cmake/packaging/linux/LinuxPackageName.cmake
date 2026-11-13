# Shared APP tarball identity for staging instructions and archive creation.
# ---------------------------------------------------------------------------
# Version
# ---------------------------------------------------------------------------
if(NOT DEFINED US3_VERSION_STRING OR US3_VERSION_STRING STREQUAL "")
    set(US3_VERSION_STRING "${PROJECT_VERSION}")
endif()

# ---------------------------------------------------------------------------
# Architecture label (x64 or arm64) for the filename
# ---------------------------------------------------------------------------
if(NOT DEFINED US3_ARCH_LABEL)
    execute_process(
        COMMAND uname -m
        OUTPUT_VARIABLE _uname_m
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(_uname_m MATCHES "aarch64|arm64")
        set(US3_ARCH_LABEL "arm64")
    else()
        set(US3_ARCH_LABEL "x64")
    endif()
endif()

set(US3_LINUX_ARCHIVE_NAME "UltraScan3-${US3_VERSION_STRING}-Linux-${US3_ARCH_LABEL}.tar.xz")
