# Auto-generated CMakeLists.txt for us_assign_reviewers_gmp

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_assign_reviewers_gmp
        ${src_dir}/us_assign_reviewers_gmp_main.cpp
        ${src_dir}/../us_esigner_gmp/us_esigner_gmp.cpp
)

target_link_libraries(us_assign_reviewers_gmp
    PRIVATE
        us_program_common
)

target_include_directories(us_assign_reviewers_gmp
    PRIVATE
        ${src_dir}
        ${src_dir}/../us_esigner_gmp
)
