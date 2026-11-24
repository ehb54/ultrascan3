# us_ddist_combine

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

# Everything but main(); other programs link these objects instead of
# compiling the sources again.
add_library(us_ddist_combine_objects OBJECT
    ${src_dir}/us_ddist_combine.cpp
    ${src_dir}/us_select_rundd.cpp
)

target_link_libraries(us_ddist_combine_objects
    PUBLIC
        us_program_common
)

target_include_directories(us_ddist_combine_objects
    PUBLIC
        ${src_dir}
)

add_executable(us_ddist_combine
    ${src_dir}/us_ddist_combine_main.cpp
)

target_link_libraries(us_ddist_combine
    PRIVATE
        us_program_common
        us_ddist_combine_objects
)
