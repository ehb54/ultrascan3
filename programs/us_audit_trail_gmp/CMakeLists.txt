# Auto-generated CMakeLists.txt for us_audit_trail_gmp

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_audit_trail_gmp
    ${src_dir}/us_audit_trail_gmp_main.cpp
    ${src_dir}/us_audit_trail_gmp.cpp
)

target_link_libraries(us_audit_trail_gmp
    PRIVATE
        us_program_common
)

target_include_directories(us_audit_trail_gmp
    PRIVATE
        ${src_dir}
)
