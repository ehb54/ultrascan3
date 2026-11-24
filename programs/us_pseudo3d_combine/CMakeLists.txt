# us_pseudo3d_combine

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

# Everything but main(), which other programs use too, goes into us_programs_results.
add_library(us_pseudo3d_combine_lib STATIC
    ${src_dir}/us_pseudo3d_combine.cpp
    ${src_dir}/us_remove_distros.cpp
)

target_link_libraries(us_pseudo3d_combine_lib
    PUBLIC
        us_program_common
)

target_include_directories(us_pseudo3d_combine_lib
    PUBLIC
        ${src_dir}
)

target_link_libraries(us_programs_results INTERFACE us_pseudo3d_combine_lib)

add_executable(us_pseudo3d_combine
    ${src_dir}/us_pseudo3d_combine_main.cpp
)

target_link_libraries(us_pseudo3d_combine
    PRIVATE
        us_program_common
        us_programs_results
)
