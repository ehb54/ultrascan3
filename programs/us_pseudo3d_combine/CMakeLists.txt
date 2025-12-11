# Auto-generated CMakeLists.txt for us_pseudo3d_combine

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_pseudo3d_combine
    ${src_dir}/us_pseudo3d_combine.cpp
    ${src_dir}/us_pseudo3d_combine_main.cpp
    ${src_dir}/us_remove_distros.cpp
)

target_link_libraries(us_pseudo3d_combine
    PRIVATE
        us_program_common
)

target_include_directories(us_pseudo3d_combine
    PRIVATE
        ${src_dir}
)
