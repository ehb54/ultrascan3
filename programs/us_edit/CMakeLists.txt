# us_edit

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

# Everything but main(), which other programs use too, goes into us_programs_data.
add_library(us_edit_lib STATIC
    ${src_dir}/us_edit.cpp
    ${src_dir}/us_select_lambdas.cpp
    ${src_dir}/us_get_edit.cpp
    ${src_dir}/us_ri_noise.cpp
    ${src_dir}/us_exclude_profile.cpp
    ${src_dir}/us_edit_scan.cpp
)

target_link_libraries(us_edit_lib
    PUBLIC
        us_program_common
)

target_include_directories(us_edit_lib
    PUBLIC
        ${src_dir}
)

target_link_libraries(us_programs_data INTERFACE us_edit_lib)

add_executable(us_edit
    ${src_dir}/us_edit_main.cpp
)

target_link_libraries(us_edit
    PRIVATE
        us_program_common
        us_programs_data
)
