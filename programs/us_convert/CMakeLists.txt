# Auto-generated CMakeLists.txt for us_convert

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_convert
    ${src_dir}/us_convert_main.cpp
)

target_link_libraries(us_convert
    PRIVATE
        us_program_common
)

target_include_directories(us_convert
    PRIVATE
        ${src_dir}
)
