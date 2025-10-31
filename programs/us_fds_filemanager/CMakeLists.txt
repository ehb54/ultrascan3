# Auto-generated CMakeLists.txt for us_fds_filemanager

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_fds_filemanager
    ${src_dir}/us_fds_filemanager.cpp
)

target_link_libraries(us_fds_filemanager
    PRIVATE
        us_program_common
)

target_include_directories(us_fds_filemanager
    PRIVATE
        ${src_dir}
)
