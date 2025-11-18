# Auto-generated CMakeLists.txt for us_helpdaemon

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_helpdaemon
    ${src_dir}/us_helpdaemon.cpp
)

target_link_libraries(us_helpdaemon
    PRIVATE
        us_program_common
)

target_include_directories(us_helpdaemon
    PRIVATE
        ${src_dir}
)
