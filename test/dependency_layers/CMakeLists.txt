# Checks that utils, gui and the programs do not depend on each other upward or
# in a cycle, that the program area libraries are consistent, and that
# the checks reject each kind of violation.

find_package(Python3 COMPONENTS Interpreter)

if(NOT Python3_Interpreter_FOUND)
    message(STATUS "test: skipping dependency layer checks (no Python 3 interpreter)")
    return()
endif()

set(DEPENDENCY_LAYERS_CHECKER ${PROJECT_SOURCE_DIR}/scripts/check-dependency-layers.py)
set(PROGRAM_LIBRARIES_CHECKER ${PROJECT_SOURCE_DIR}/scripts/check-program-libraries.py)

add_test(NAME dependency_layers
        COMMAND ${Python3_EXECUTABLE} ${DEPENDENCY_LAYERS_CHECKER} --root ${PROJECT_SOURCE_DIR})

add_test(NAME dependency_layers_rejects_violations
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_dependency_layers.py ${DEPENDENCY_LAYERS_CHECKER})

add_test(NAME program_libraries
        COMMAND ${Python3_EXECUTABLE} ${PROGRAM_LIBRARIES_CHECKER} --root ${PROJECT_SOURCE_DIR})

add_test(NAME program_libraries_rejects_violations
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_program_libraries.py ${PROGRAM_LIBRARIES_CHECKER})

set_tests_properties(dependency_layers dependency_layers_rejects_violations
        program_libraries program_libraries_rejects_violations PROPERTIES
        LABELS "dependency_layers"
        TIMEOUT 60)
