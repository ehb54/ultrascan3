# Auto-generated CMakeLists.txt for us_modelmetrics

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_modelmetrics
    ${src_dir}/us_modelmetrics.cpp
)

target_link_libraries(us_modelmetrics
    PRIVATE
        us_program_common
)

target_include_directories(us_modelmetrics
    PRIVATE
        ${src_dir}
)
