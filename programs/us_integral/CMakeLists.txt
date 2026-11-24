# us_integral

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

# Everything but main(); other programs link these objects instead of
# compiling the sources again.
add_library(us_integral_objects OBJECT
    ${src_dir}/us_integral.cpp
    ${src_dir}/us_delete_models.cpp
)

target_link_libraries(us_integral_objects
    PUBLIC
        us_program_common
)

target_include_directories(us_integral_objects
    PUBLIC
        ${src_dir}
)

add_executable(us_integral
    ${src_dir}/us_integral_main.cpp
)

target_link_libraries(us_integral
    PRIVATE
        us_program_common
        us_integral_objects
)
