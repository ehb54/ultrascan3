# Auto-generated CMakeLists.txt for us_manage_data

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_manage_data
    ${src_dir}/us_sync_exper.cpp
    ${src_dir}/us_data_process.cpp
    ${src_dir}/us_data_model.cpp
    ${src_dir}/us_data_tree.cpp
    ${src_dir}/us_manage_data.cpp
)

target_link_libraries(us_manage_data
    PRIVATE
        us_program_common
)

target_include_directories(us_manage_data
    PRIVATE
        ${src_dir}
)
