# Command-line validation tests for us_sim_inputs_gen.
if(TARGET us_sim_inputs_gen)
    us3_add_gui_test(test_us_sim_inputs_gen_cli
            SOURCES
                test_us_sim_inputs_gen_cli.cpp
            LIBRARIES
                UltraScan3::Gui_Static
            LABELS
                programs;simulation
            )

    add_dependencies(test_us_sim_inputs_gen_cli us_sim_inputs_gen)

    target_compile_definitions(test_us_sim_inputs_gen_cli PRIVATE
            US_SIM_INPUTS_GEN_EXE="$<TARGET_FILE:us_sim_inputs_gen>")
else()
    message(STATUS
        "test: skipping test_us_sim_inputs_gen_cli (us_sim_inputs_gen is not built)")
endif()
