# Auto-generated CMakeLists.txt for us_pcsa

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_pcsa
    ${src_dir}/us_adv_analysis_pc.cpp
    ${src_dir}/us_pcsa.cpp
    ${src_dir}/us_plot_control_pc.cpp
    ${src_dir}/us_pcsa_process.cpp
    ${src_dir}/us_resplot_pc.cpp
    ${src_dir}/us_mrecs_loader.cpp
    ${src_dir}/us_analysis_control_pc.cpp
    ${src_dir}/us_mlplot.cpp
    ${src_dir}/us_rpscan.cpp
    ${src_dir}/us_worker_pc.cpp
)

target_link_libraries(us_pcsa
    PRIVATE
        us_program_common
)

target_include_directories(us_pcsa
    PRIVATE
        ${src_dir}
)
