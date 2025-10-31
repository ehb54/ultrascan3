# Auto-generated CMakeLists.txt for us

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us
    ${src_dir}/us_win_data.cpp
    ${src_dir}/us.cpp
)

target_link_libraries(us
    PRIVATE
        us_program_common
)

target_include_directories(us
    PRIVATE
        ${src_dir}
)
