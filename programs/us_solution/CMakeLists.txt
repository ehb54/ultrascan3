# Auto-generated CMakeLists.txt for us_solution

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_solution
    ${src_dir}/us_solution_main.cpp
)

target_link_libraries(us_solution
    PRIVATE
        us_program_common
)

target_include_directories(us_solution
    PRIVATE
        ${src_dir}
)
