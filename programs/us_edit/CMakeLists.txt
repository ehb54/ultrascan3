# Auto-generated CMakeLists.txt for us_edit

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_edit
    ${src_dir}/us_edit_main.cpp
    ${src_dir}/us_edit.cpp
    ${src_dir}/us_select_lambdas.cpp
    ${src_dir}/us_get_edit.cpp
    ${src_dir}/us_ri_noise.cpp
    ${src_dir}/us_exclude_profile.cpp
    ${src_dir}/us_edit_scan.cpp
)

target_link_libraries(us_edit
    PRIVATE
        us_program_common
)

target_include_directories(us_edit
    PRIVATE
        ${src_dir}
)
