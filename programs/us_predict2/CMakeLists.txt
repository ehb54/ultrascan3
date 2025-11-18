# Auto-generated CMakeLists.txt for us_predict2

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_predict2
    ${src_dir}/us_predict2.cpp
    ${src_dir}/us_predict2_main.cpp
)

target_link_libraries(us_predict2
    PRIVATE
        us_program_common
)

target_include_directories(us_predict2
    PRIVATE
        ${src_dir}
)
