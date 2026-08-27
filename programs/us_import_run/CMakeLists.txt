# us_import_run is a headless command-line tool. US_ConvertIO, US_Experiment,
# and US_Solution all live in utils, so the database write path needs no
# Widgets or GUI libraries.

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_import_run
    ${src_dir}/us_import_run.cpp
)

target_link_libraries(us_import_run
    PRIVATE
        UltraScan3::Utils
)

target_include_directories(us_import_run
    PRIVATE
        ${src_dir}
)
