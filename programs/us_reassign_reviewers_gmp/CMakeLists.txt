# Auto-generated CMakeLists.txt for us_reassign_reviewers_gmp

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_reassign_reviewers_gmp
        ${src_dir}/us_reassign_reviewers_gmp_main.cpp
)

target_link_libraries(us_reassign_reviewers_gmp
    PRIVATE
        us_program_common
        us_programs_gmp
)

target_include_directories(us_reassign_reviewers_gmp
    PRIVATE
        ${src_dir}
)
