# Auto-generated CMakeLists.txt for us_integral

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_integral
    ${src_dir}/us_integral.cpp
    ${src_dir}/us_delete_models.cpp
)

target_link_libraries(us_integral
    PRIVATE
        us_program_common
)

target_include_directories(us_integral
    PRIVATE
        ${src_dir}
)
