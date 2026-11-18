# Argument, staging and pre-connection checks for us_import_run.
if(TARGET us_import_run)
    us3_add_gui_test(test_us_import_run_cli
            SOURCES
                test_us_import_run_cli.cpp
                ${CMAKE_SOURCE_DIR}/test/utils/support/disk_run_fixture.cpp
            LIBRARIES
                UltraScan3::Gui_Static
            LABELS
                programs;import
            )

    add_dependencies(test_us_import_run_cli us_import_run)

    # disk_run_fixture writes the synthetic runs.
    target_include_directories(test_us_import_run_cli PRIVATE
            ${CMAKE_SOURCE_DIR}/test/utils/support)

    target_compile_definitions(test_us_import_run_cli PRIVATE
            US_IMPORT_RUN_EXE="$<TARGET_FILE:us_import_run>")
else()
    message(STATUS
        "test: skipping test_us_import_run_cli (us_import_run is not built)")
endif()
