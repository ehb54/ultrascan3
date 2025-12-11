# Auto-generated CMakeLists.txt for us_reporter

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_reporter
    ${src_dir}/us_sync_db.cpp
    ${src_dir}/us_reporter.cpp
)

target_link_libraries(us_reporter
    PRIVATE
        us_program_common
)

target_include_directories(us_reporter
    PRIVATE
        ${src_dir}
)
