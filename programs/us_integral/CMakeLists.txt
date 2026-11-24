# us_integral

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

# Everything but main(), which other programs use too, goes into us_programs_results.
add_library(us_integral_lib STATIC
    ${src_dir}/us_integral.cpp
    ${src_dir}/us_delete_models.cpp
)

target_link_libraries(us_integral_lib
    PUBLIC
        us_program_common
)

target_include_directories(us_integral_lib
    PUBLIC
        ${src_dir}
)

target_link_libraries(us_programs_results INTERFACE us_integral_lib)

add_executable(us_integral
    ${src_dir}/us_integral_main.cpp
)

target_link_libraries(us_integral
    PRIVATE
        us_program_common
        us_programs_results
)
