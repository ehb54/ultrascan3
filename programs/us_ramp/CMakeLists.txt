# Auto-generated CMakeLists.txt for us_ramp

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_ramp
    ${src_dir}/us_experiment_gui_ra.cpp
    ${src_dir}/us_get_dbrun_ra.cpp
    ${src_dir}/us_ramp.cpp
    ${src_dir}/us_ramp_gui.cpp
    ${src_dir}/us_selectbox_ra.cpp
    ${src_dir}/us_mwlramp.cpp
    ${src_dir}/us_intensity_ra.cpp
    ${src_dir}/us_select_triples_ra.cpp
    ${src_dir}/us_experiment_ra.cpp
)

target_link_libraries(us_ramp
    PRIVATE
        us_program_common
)

target_include_directories(us_ramp
    PRIVATE
        ${src_dir}
)
