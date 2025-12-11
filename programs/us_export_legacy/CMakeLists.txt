# Auto-generated CMakeLists.txt for us_export_legacy

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_export_legacy
    ${src_dir}/us_export.cpp
)

target_link_libraries(us_export_legacy
    PRIVATE
        us_program_common
)

target_include_directories(us_export_legacy
    PRIVATE
        ${src_dir}
)
