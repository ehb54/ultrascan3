# Auto-generated CMakeLists.txt for us_2dsa

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_2dsa
    ${src_dir}/us_plot_control_2d.cpp
    ${src_dir}/us_2dsa.cpp
    ${src_dir}/us_adv_analysis_2d.cpp
    ${src_dir}/us_worker_calcnorm.cpp
    ${src_dir}/us_2dsa_process.cpp
    ${src_dir}/us_resplot_2d.cpp
    ${src_dir}/us_worker_2d.cpp
    ${src_dir}/us_show_norm.cpp
    ${src_dir}/us_analysis_control_2d.cpp
)

target_link_libraries(us_2dsa
    PRIVATE
        us_program_common
)

target_include_directories(us_2dsa
    PRIVATE
        ${src_dir}
)
