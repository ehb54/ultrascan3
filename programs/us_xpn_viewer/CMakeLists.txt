# us_xpn_viewer

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

# Everything but main(), which other programs use too, goes into us_programs_data.
add_library(us_xpn_viewer_lib STATIC
    ${src_dir}/us_xpn_viewer_gui.cpp
    ${src_dir}/us_xpn_run_auc.cpp
    ${src_dir}/us_xpn_run_raw.cpp
)

target_link_libraries(us_xpn_viewer_lib
    PUBLIC
        us_program_common
)

target_include_directories(us_xpn_viewer_lib
    PUBLIC
        ${src_dir}
)

target_link_libraries(us_programs_data INTERFACE us_xpn_viewer_lib)

add_executable(us_xpn_viewer
    ${src_dir}/us_xpn_viewer_main.cpp
)

target_link_libraries(us_xpn_viewer
    PRIVATE
        us_program_common
        us_programs_data
)
