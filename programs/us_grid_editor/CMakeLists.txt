# Auto-generated CMakeLists.txt for us_grid_editor

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_grid_editor
    ${src_dir}/us_grid_editor.cpp
)

target_link_libraries(us_grid_editor
    PRIVATE
        us_program_common
)

target_include_directories(us_grid_editor
    PRIVATE
        ${src_dir}
)
