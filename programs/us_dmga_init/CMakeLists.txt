# Auto-generated CMakeLists.txt for us_dmga_init

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_dmga_init
    ${src_dir}/us_constraints_edit.cpp
    ${src_dir}/us_dmga_init.cpp
)

target_link_libraries(us_dmga_init
    PRIVATE
        us_program_common
)

target_include_directories(us_dmga_init
    PRIVATE
        ${src_dir}
)
