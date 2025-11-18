# Auto-generated CMakeLists.txt for us_config

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_config
    ${src_dir}/us_color.cpp
    ${src_dir}/us_xpnhost_db.cpp
    ${src_dir}/us_xpnhost.cpp
    ${src_dir}/us_font.cpp
    ${src_dir}/us_advanced.cpp
    ${src_dir}/us_admin.cpp
    ${src_dir}/us_database.cpp
    ${src_dir}/us_config.cpp
    ${src_dir}/us_newxpnhost_db.cpp
)

target_link_libraries(us_config
    PRIVATE
        us_program_common
)

target_include_directories(us_config
    PRIVATE
        ${src_dir}
)
