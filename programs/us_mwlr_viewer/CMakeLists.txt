# Auto-generated CMakeLists.txt for us_mwlr_viewer

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_mwlr_viewer
    ${src_dir}/us_mwlr_viewer.cpp
    ${src_dir}/us_mwl_run.cpp
    ${src_dir}/us_mwl_pltctrl.cpp
)

target_link_libraries(us_mwlr_viewer
    PRIVATE
        us_program_common
)

target_include_directories(us_mwlr_viewer
    PRIVATE
        ${src_dir}
)
