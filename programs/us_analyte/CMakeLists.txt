# Auto-generated CMakeLists.txt for us_analyte

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_analyte
    ${src_dir}/us_analyte_main.cpp
)

set_target_properties(us_analyte PROPERTIES
    OUTPUT_NAME us_analyte_gui
)

target_link_libraries(us_analyte
    PRIVATE
        us_program_common
)

target_include_directories(us_analyte
    PRIVATE
        ${src_dir}
)
