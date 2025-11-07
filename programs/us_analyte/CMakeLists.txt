# Auto-generated CMakeLists.txt for us_analyte

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_analyte
    ${src_dir}/us_analyte_main.cpp
)

target_link_libraries(us_analyte
    PRIVATE
        us_program_common
)

target_include_directories(us_analyte
    PRIVATE
        ${src_dir}
)
