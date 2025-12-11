# Auto-generated CMakeLists.txt for us_combine_models

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_combine_models
    ${src_dir}/us_combine_models.cpp
)

target_link_libraries(us_combine_models
    PRIVATE
        us_program_common
)

target_include_directories(us_combine_models
    PRIVATE
        ${src_dir}
)
