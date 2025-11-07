# Auto-generated CMakeLists.txt for us_esigner_gmp

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_esigner_gmp
    ${src_dir}/us_esigner_gmp_main.cpp
    ${src_dir}/us_esigner_gmp.cpp
)

target_link_libraries(us_esigner_gmp
    PRIVATE
        us_program_common
)

target_include_directories(us_esigner_gmp
    PRIVATE
        ${src_dir}
)
