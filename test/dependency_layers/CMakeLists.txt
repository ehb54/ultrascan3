# Checks that utils, gui and the programs do not depend on each other upward or
# in a cycle, that each program source is compiled by one directory, and that
# the checks reject each kind of violation.

find_package(Python3 COMPONENTS Interpreter)

if(NOT Python3_Interpreter_FOUND)
    message(STATUS "test: skipping dependency layer checks (no Python 3 interpreter)")
    return()
endif()

set(DEPENDENCY_LAYERS_CHECKER ${PROJECT_SOURCE_DIR}/scripts/check-dependency-layers.py)
set(DUPLICATE_SOURCES_CHECKER ${PROJECT_SOURCE_DIR}/scripts/check-duplicate-sources.py)

add_test(NAME dependency_layers
        COMMAND ${Python3_EXECUTABLE} ${DEPENDENCY_LAYERS_CHECKER} --root ${PROJECT_SOURCE_DIR})

add_test(NAME dependency_layers_rejects_violations
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_dependency_layers.py ${DEPENDENCY_LAYERS_CHECKER})

add_test(NAME duplicate_sources
        COMMAND ${Python3_EXECUTABLE} ${DUPLICATE_SOURCES_CHECKER} --root ${PROJECT_SOURCE_DIR})

add_test(NAME duplicate_sources_rejects_violations
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_duplicate_sources.py ${DUPLICATE_SOURCES_CHECKER})

set_tests_properties(dependency_layers dependency_layers_rejects_violations
        duplicate_sources duplicate_sources_rejects_violations PROPERTIES
        LABELS "dependency_layers"
        TIMEOUT 60)
