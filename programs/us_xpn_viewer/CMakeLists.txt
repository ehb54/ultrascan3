# us_xpn_viewer

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

# Everything but main(); other programs link these objects instead of
# compiling the sources again.
add_library(us_xpn_viewer_objects OBJECT
    ${src_dir}/us_xpn_viewer_gui.cpp
    ${src_dir}/us_xpn_run_auc.cpp
    ${src_dir}/us_xpn_run_raw.cpp
)

target_link_libraries(us_xpn_viewer_objects
    PUBLIC
        us_program_common
)

target_include_directories(us_xpn_viewer_objects
    PUBLIC
        ${src_dir}
)

add_executable(us_xpn_viewer
    ${src_dir}/us_xpn_viewer_main.cpp
)

target_link_libraries(us_xpn_viewer
    PRIVATE
        us_program_common
        us_xpn_viewer_objects
)
