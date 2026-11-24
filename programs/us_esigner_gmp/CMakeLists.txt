# us_esigner_gmp

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

# Everything but main(); other programs link these objects instead of
# compiling the sources again.
add_library(us_esigner_gmp_objects OBJECT
    ${src_dir}/us_esigner_gmp.cpp
)

target_link_libraries(us_esigner_gmp_objects
    PUBLIC
        us_program_common
)

target_include_directories(us_esigner_gmp_objects
    PUBLIC
        ${src_dir}
)

add_executable(us_esigner_gmp
    ${src_dir}/us_esigner_gmp_main.cpp
)

target_link_libraries(us_esigner_gmp
    PRIVATE
        us_program_common
        us_esigner_gmp_objects
)
