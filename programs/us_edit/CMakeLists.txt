# us_edit

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

# Everything but main(); other programs link these objects instead of
# compiling the sources again.
add_library(us_edit_objects OBJECT
    ${src_dir}/us_edit.cpp
    ${src_dir}/us_select_lambdas.cpp
    ${src_dir}/us_get_edit.cpp
    ${src_dir}/us_ri_noise.cpp
    ${src_dir}/us_exclude_profile.cpp
    ${src_dir}/us_edit_scan.cpp
)

target_link_libraries(us_edit_objects
    PUBLIC
        us_program_common
)

target_include_directories(us_edit_objects
    PUBLIC
        ${src_dir}
)

add_executable(us_edit
    ${src_dir}/us_edit_main.cpp
)

target_link_libraries(us_edit
    PRIVATE
        us_program_common
        us_edit_objects
)
