# Auto-generated CMakeLists.txt for us_tmst_viewer

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_tmst_viewer
    ${src_dir}/us_tmst_viewer.cpp
)

target_link_libraries(us_tmst_viewer
    PRIVATE
        us_program_common
)

target_include_directories(us_tmst_viewer
    PRIVATE
        ${src_dir}
)
