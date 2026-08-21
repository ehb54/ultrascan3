# Compile the same simulation dialog implementation used by us_astfem_sim.
# Its source is already separate from the application's main() function, so
# the production program does not need a test-only code path.
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
