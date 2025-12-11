# Auto-generated CMakeLists.txt for us_equiltime

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_equiltime
    ${src_dir}/us_equiltime.cpp
)

target_link_libraries(us_equiltime
    PRIVATE
        us_program_common
)

target_include_directories(us_equiltime
    PRIVATE
        ${src_dir}
)
