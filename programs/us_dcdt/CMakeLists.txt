# Auto-generated CMakeLists.txt for us_dcdt

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_dcdt
    ${src_dir}/us_dcdt.cpp
)

target_link_libraries(us_dcdt
    PRIVATE
        us_program_common
)

target_include_directories(us_dcdt
    PRIVATE
        ${src_dir}
)
