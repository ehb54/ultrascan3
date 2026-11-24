# programs/us_abde/CMakeLists.txt

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

# Everything but main(), which other programs use too, goes into us_programs_results.
add_library(us_abde_lib STATIC
        ${src_dir}/us_norm_profile.cpp
        )

target_link_libraries(us_abde_lib
        PUBLIC
        us_program_common
        )

target_include_directories(us_abde_lib
        PUBLIC
        ${src_dir}
        )

target_link_libraries(us_programs_results INTERFACE us_abde_lib)

add_executable(us_abde
        ${src_dir}/us_abde_main.cpp
        )

target_link_libraries(us_abde
        PRIVATE
        us_program_common
        us_programs_results
        )
