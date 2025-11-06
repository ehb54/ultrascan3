# Auto-generated CMakeLists.txt for us_second_moment

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_second_moment
    ${src_dir}/us_second_moment.cpp
)

target_link_libraries(us_second_moment
    PRIVATE
        us_program_common
)

target_include_directories(us_second_moment
    PRIVATE
        ${src_dir}
)
