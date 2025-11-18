# Auto-generated CMakeLists.txt for us_license

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_license
    ${src_dir}/us_license.cpp
)

target_link_libraries(us_license
    PRIVATE
        us_program_common
)

target_include_directories(us_license
    PRIVATE
        ${src_dir}
)
