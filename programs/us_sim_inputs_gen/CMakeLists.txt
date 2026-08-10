# us_sim_inputs_gen: headless, no-GUI CLI tool. Links only Qt::Core/Xml
# (via UltraScan3::Utils's own PUBLIC deps) -- no us_program_common, no
# Widgets/Gui stack, since US_SimInputs only touches plain data classes.

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
