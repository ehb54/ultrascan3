# Auto-generated CMakeLists.txt for us_project

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_project
    ${src_dir}/us_project_main.cpp
)

target_link_libraries(us_project
    PRIVATE
        us_program_common
)

target_include_directories(us_project
    PRIVATE
        ${src_dir}
)
