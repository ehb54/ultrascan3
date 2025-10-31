# Auto-generated CMakeLists.txt for us_colorgradient

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_colorgradient
    ${src_dir}/us_colorgradient.cpp
)

target_link_libraries(us_colorgradient
    PRIVATE
        us_program_common
)

target_include_directories(us_colorgradient
    PRIVATE
        ${src_dir}
)
