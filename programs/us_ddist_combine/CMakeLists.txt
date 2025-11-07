# Auto-generated CMakeLists.txt for us_ddist_combine

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_ddist_combine
    ${src_dir}/us_ddist_combine_main.cpp
    ${src_dir}/us_ddist_combine.cpp
    ${src_dir}/us_select_rundd.cpp
)

target_link_libraries(us_ddist_combine
    PRIVATE
        us_program_common
)

target_include_directories(us_ddist_combine
    PRIVATE
        ${src_dir}
)
