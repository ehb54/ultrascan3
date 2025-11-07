# Auto-generated CMakeLists.txt for us_buffer

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_buffer
    ${src_dir}/us_buffer_main.cpp
)

target_link_libraries(us_buffer
    PRIVATE
        us_program_common
)

target_include_directories(us_buffer
    PRIVATE
        ${src_dir}
)
