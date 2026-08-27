# Shared support for native Qt Test executables.  Each UI test supplies its
# own QObject test class and uses US3_GUI_TEST_MAIN() to create one sandboxed
# QApplication before any widgets are constructed.
add_library(us3_gui_test_support INTERFACE)
target_include_directories(us3_gui_test_support INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/common
        )
target_link_libraries(us3_gui_test_support INTERFACE
        Qt::Test
        Qt::Widgets
        )

function(us3_add_gui_test target)
    set(options)
    set(one_value_args)
    set(multi_value_args SOURCES LIBRARIES INCLUDE_DIRECTORIES LABELS)
    cmake_parse_arguments(TEST
            "${options}"
            "${one_value_args}"
            "${multi_value_args}"
            ${ARGN}
            )

    add_executable(${target} ${TEST_SOURCES})
    target_link_libraries(${target} PRIVATE
            us3_gui_test_support
            ${TEST_LIBRARIES}
            )

    if(TEST_INCLUDE_DIRECTORIES)
        target_include_directories(${target} PRIVATE
                ${TEST_INCLUDE_DIRECTORIES}
                )
    endif()

    add_test(NAME ${target} COMMAND ${target})
    set_tests_properties(${target} PROPERTIES
            LABELS "qtest;headless;${TEST_LABELS}"
            TIMEOUT 60
            ENVIRONMENT
                "QT_QPA_PLATFORM=offscreen;QT_OPENGL=software;LIBGL_ALWAYS_SOFTWARE=1"
            )
endfunction()

add_subdirectory(utils)

# The GUI library and the programs that link it are only built when the tree has
# database support (see the top-level CMakeLists.txt), so their tests exist only
# in the DB-enabled variant.  The NO_DB variant still configures and runs the
# utilities tests.
if(TARGET UltraScan3::Gui_Static)
    add_subdirectory(gui)
else()
    message(STATUS "test: skipping GUI and program tests (no UltraScan3::Gui_Static in this variant)")
endif()
