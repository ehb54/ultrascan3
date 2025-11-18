# Auto-generated CMakeLists.txt for us_convert

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_convert
    ${src_dir}/us_convert_gui.cpp
    ${src_dir}/us_convert_main.cpp
    ${src_dir}/us_convertio.cpp
    ${src_dir}/us_experiment_gui.cpp
    ${src_dir}/us_intensity.cpp
    ${src_dir}/us_convert.cpp
    ${src_dir}/us_select_triples.cpp
    ${src_dir}/us_get_run.cpp
    ${src_dir}/us_selectbox.cpp
    ${src_dir}/us_experiment.cpp
)

target_link_libraries(us_convert
    PRIVATE
        us_program_common
)

target_include_directories(us_convert
    PRIVATE
        ${src_dir}
)
