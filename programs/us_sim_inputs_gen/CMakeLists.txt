# us_sim_inputs_gen is a headless command-line tool. It uses the Qt Core and
# XML dependencies exposed by UltraScan3::Utils and does not require the
# Widgets or GUI libraries.

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_sim_inputs_gen
    ${src_dir}/us_sim_inputs_gen.cpp
)

target_link_libraries(us_sim_inputs_gen
    PRIVATE
        UltraScan3::Utils
)

target_include_directories(us_sim_inputs_gen
    PRIVATE
        ${src_dir}
)
