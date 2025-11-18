# Auto-generated CMakeLists.txt for us_ga_init

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_ga_init
    ${src_dir}/us_ga_init.cpp
    ${src_dir}/us_solutedata.cpp
)

target_link_libraries(us_ga_init
    PRIVATE
        us_program_common
)

target_include_directories(us_ga_init
    PRIVATE
        ${src_dir}
)
