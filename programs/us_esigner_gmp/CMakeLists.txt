# us_esigner_gmp

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

# Everything but main(), which other programs use too, goes into us_programs_gmp.
add_library(us_esigner_gmp_lib STATIC
    ${src_dir}/us_esigner_gmp.cpp
)

target_link_libraries(us_esigner_gmp_lib
    PUBLIC
        us_program_common
)

target_include_directories(us_esigner_gmp_lib
    PUBLIC
        ${src_dir}
)

target_link_libraries(us_programs_gmp INTERFACE us_esigner_gmp_lib)

add_executable(us_esigner_gmp
    ${src_dir}/us_esigner_gmp_main.cpp
)

target_link_libraries(us_esigner_gmp
    PRIVATE
        us_program_common
        us_programs_gmp
)
