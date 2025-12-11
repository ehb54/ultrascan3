# Auto-generated CMakeLists.txt for us_sassoc

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_sassoc
    ${src_dir}/us_sassoc_main.cpp
)

target_link_libraries(us_sassoc
    PRIVATE
        us_program_common
)

target_include_directories(us_sassoc
    PRIVATE
        ${src_dir}
)
