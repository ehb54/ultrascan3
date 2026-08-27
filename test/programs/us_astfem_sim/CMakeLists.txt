# Tests for the ASTFEM simulator and its supporting data transforms.
us3_add_gui_test(test_us_astfem_clipdata_gui
        SOURCES
            test_us_astfem_clipdata_gui.cpp
            ${CMAKE_SOURCE_DIR}/programs/us_astfem_sim/us_clipdata.cpp
            ${CMAKE_SOURCE_DIR}/programs/us_astfem_sim/us_clipdata.h
        LIBRARIES
            UltraScan3::Gui_Static
        INCLUDE_DIRECTORIES
            ${CMAKE_SOURCE_DIR}/programs/us_astfem_sim
        LABELS
            programs;simulation
        )

us3_add_gui_test(test_us_astfem_noise_gui
        SOURCES
            test_us_astfem_noise_gui.cpp
            ${CMAKE_SOURCE_DIR}/programs/us_astfem_sim/us_astfem_sim.cpp
            ${CMAKE_SOURCE_DIR}/programs/us_astfem_sim/us_astfem_sim.h
            ${CMAKE_SOURCE_DIR}/programs/us_astfem_sim/us_clipdata.cpp
            ${CMAKE_SOURCE_DIR}/programs/us_astfem_sim/us_clipdata.h
        LIBRARIES
            UltraScan3::Gui_Static
        INCLUDE_DIRECTORIES
            ${CMAKE_SOURCE_DIR}/programs
            ${CMAKE_SOURCE_DIR}/programs/us_astfem_sim
        LABELS
            programs;simulation;headless
        )

target_compile_definitions(test_us_astfem_noise_gui
        PRIVATE US_ASTFEM_SIM_NO_MAIN)

# The persisted layout of a multi-speed run is a contract about files, so this
# one drives the built simulator rather than its classes.  It therefore exists
# only where the programs are built too, which the test-only profile is not.
if(TARGET us_astfem_sim AND TARGET us_sim_inputs_gen)
    us3_add_gui_test(test_us_astfem_persist_gui
            SOURCES
                test_us_astfem_persist_gui.cpp
            LIBRARIES
                UltraScan3::Gui_Static
            LABELS
                programs;simulation;headless
            )

    add_dependencies(test_us_astfem_persist_gui us_astfem_sim us_sim_inputs_gen)

    target_compile_definitions(test_us_astfem_persist_gui PRIVATE
            US_ASTFEM_SIM_EXE="$<TARGET_FILE:us_astfem_sim>"
            US_SIM_INPUTS_GEN_EXE="$<TARGET_FILE:us_sim_inputs_gen>")

    # It runs a real simulation in a child process.
    set_tests_properties(test_us_astfem_persist_gui PROPERTIES TIMEOUT 600)
else()
    message(STATUS
        "test: skipping test_us_astfem_persist_gui (simulator programs are not built)")
endif()
