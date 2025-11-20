# Auto-generated CMakeLists.txt for us_legacy_converter

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_legacy_converter
        ${src_dir}/us_legacy_converter.cpp
)

target_link_libraries(us_legacy_converter
    PRIVATE
        us_program_common
)

target_include_directories(us_legacy_converter
    PRIVATE
        ${src_dir}
)
