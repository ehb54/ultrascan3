# Auto-generated CMakeLists.txt for us_vhw_combine

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_vhw_combine
    ${src_dir}/us_vhwc_pltctl.cpp
    ${src_dir}/us_vhw_combine.cpp
    ${src_dir}/us_select_runid.cpp
)

target_link_libraries(us_vhw_combine
    PRIVATE
        us_program_common
)

target_include_directories(us_vhw_combine
    PRIVATE
        ${src_dir}
)
