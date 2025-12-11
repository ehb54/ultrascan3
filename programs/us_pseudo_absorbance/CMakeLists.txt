# Auto-generated CMakeLists.txt for us_pseudo_absorbance

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_pseudo_absorbance
    ${src_dir}/us_pabs_common.cpp
    ${src_dir}/us_pseudo_absorbance.cpp
    ${src_dir}/us_remove_ri.cpp
    ${src_dir}/us_refScan_dataIO.cpp
    ${src_dir}/us_add_refScan.cpp
    ${src_dir}/us_convert_scan.cpp
)

target_link_libraries(us_pseudo_absorbance
    PRIVATE
        us_program_common
)

target_include_directories(us_pseudo_absorbance
    PRIVATE
        ${src_dir}
)
