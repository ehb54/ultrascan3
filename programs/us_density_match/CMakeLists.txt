# Auto-generated CMakeLists.txt for us_density_match

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_density_match
    ${src_dir}/us_remove_models.cpp
    ${src_dir}/us_model_params.cpp
    ${src_dir}/us_density_match.cpp
)

target_link_libraries(us_density_match
    PRIVATE
        us_program_common
)

target_include_directories(us_density_match
    PRIVATE
        ${src_dir}
)
