# Auto-generated CMakeLists.txt for us_vhw_enhanced

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_vhw_enhanced
    ${src_dir}/us_vhw_enhanced.cpp
    ${src_dir}/us_distrib_plot.cpp
)

target_link_libraries(us_vhw_enhanced
    PRIVATE
        us_program_common
)

target_include_directories(us_vhw_enhanced
    PRIVATE
        ${src_dir}
)
