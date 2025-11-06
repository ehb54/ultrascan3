# Auto-generated CMakeLists.txt for us_2dplot

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_2dplot
    ${src_dir}/us_2dplot.cpp
)

target_link_libraries(us_2dplot
    PRIVATE
        us_program_common
)

target_include_directories(us_2dplot
    PRIVATE
        ${src_dir}
)
