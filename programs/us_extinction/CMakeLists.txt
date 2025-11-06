# Auto-generated CMakeLists.txt for us_extinction

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_extinction
    ${src_dir}/us_extinction_main.cpp
)

target_link_libraries(us_extinction
    PRIVATE
        us_program_common
)

target_include_directories(us_extinction
    PRIVATE
        ${src_dir}
)
