# Auto-generated CMakeLists.txt for us

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us
    ${src_dir}/us_win_data.cpp
    ${src_dir}/us.cpp
)

target_link_libraries(us
    PRIVATE
        us_program_common
)

# Ensure us_revision.h is found
target_include_directories(us PRIVATE
        ${CMAKE_BINARY_DIR}/us       # where us_revision.h lives
        )

# Ensure version headers are always generated first
add_dependencies(us us_version_headers)

target_include_directories(us
    PRIVATE
        ${src_dir}
)
