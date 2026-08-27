# Compile the same class implementation used by us_mwl_species_sim. Its source
# is separate from the application's main() function, so the production program
# does not need a test-only code path.
us3_add_gui_test(test_us_mwl_species_sim_editnames_gui
        SOURCES
            test_us_mwl_species_sim_editnames_gui.cpp
            ${CMAKE_SOURCE_DIR}/programs/us_mwl_species_sim/us_mwl_species_sim.cpp
            ${CMAKE_SOURCE_DIR}/programs/us_mwl_species_sim/us_mwl_species_sim.h
        LIBRARIES
            UltraScan3::Gui_Static
        INCLUDE_DIRECTORIES
            ${CMAKE_SOURCE_DIR}/programs
            ${CMAKE_SOURCE_DIR}/programs/us_mwl_species_sim
        LABELS
            programs;simulation
        )

# What the program writes per wavelength is a contract about files, so this one
# drives the built simulator.  It therefore exists only where the programs are
# built too, which the test-only profile is not.
if(TARGET us_mwl_species_sim AND TARGET us_sim_inputs_gen)
    us3_add_gui_test(test_us_mwl_species_sim_persist_gui
            SOURCES
                test_us_mwl_species_sim_persist_gui.cpp
            LIBRARIES
                UltraScan3::Gui_Static
            LABELS
                programs;simulation;headless
            )

    add_dependencies(test_us_mwl_species_sim_persist_gui
            us_mwl_species_sim us_sim_inputs_gen)

    target_compile_definitions(test_us_mwl_species_sim_persist_gui PRIVATE
            US_MWL_SPECIES_SIM_EXE="$<TARGET_FILE:us_mwl_species_sim>"
            US_SIM_INPUTS_GEN_EXE="$<TARGET_FILE:us_sim_inputs_gen>")

    # It runs a real simulation in a child process.
    set_tests_properties(test_us_mwl_species_sim_persist_gui
            PROPERTIES TIMEOUT 600)
else()
    message(STATUS
        "test: skipping test_us_mwl_species_sim_persist_gui (simulator programs are not built)")
endif()
