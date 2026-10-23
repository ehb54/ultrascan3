# Regression tests for us_mpi_analysis.
#
# us_mpi_testdata writes the synthetic datasets; us_mpi_regression.py builds
# the job archives, runs us_mpi_analysis under mpirun and compares the fitted
# models with the stored baselines in baseline/.

find_package(Python3 COMPONENTS Interpreter)

if(TARGET UltraScan3::Utils_Static)
    set(US_MPI_TEST_UTILS UltraScan3::Utils_Static)
else()
    set(US_MPI_TEST_UTILS UltraScan3::Utils)
endif()

add_executable(us_mpi_testdata us_mpi_testdata.cpp)
target_link_libraries(us_mpi_testdata PRIVATE ${US_MPI_TEST_UTILS} Qt::Core Qt::Xml)

if(NOT Python3_Interpreter_FOUND)
    message(STATUS "test/mpi_analysis: Python3 not found, regression tests skipped")
    return()
endif()

set(US_MPI_CASES
    2dsa_basic
    2dsa_refine
    2dsa_mc
    2dsa_mc_pmasters
    2dsa_mc_pmasters3
    2dsa_mc_pmasters_idle
    2dsa_mc_pmasters_uneven
    2dsa_mc_odd
    2dsa_mc_odd_pmasters
    2dsa_meniscus
    2dsa_meniscus_nobottom
    2dsa_bottom
    2dsa_menbot
    2dsa_composite_menbot
    2dsa_composite_menbot_pmasters
    2dsa_noise
    2dsa_customgrid
    2dsa_global
    2dsa_global_mc
    2dsa_composite
    2dsa_composite_pmasters
    2dsa_composite_pmasters_np4
    2dsa_composite_mc
    2dsa_composite_mc_pmasters
    2dsa_composite_meniscus
    2dsa_composite_meniscus_pmasters
    2dsa_refine_np4
    2dsa_mc_np4
    pcsa_sl
    ga_basic
)

foreach(case ${US_MPI_CASES})
    add_test(NAME mpi_analysis_${case}
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/us_mpi_regression.py
            check ${case}
            --program $<TARGET_FILE:us_mpi_analysis>
            --testdata $<TARGET_FILE:us_mpi_testdata>
            --mpirun ${MPIEXEC_EXECUTABLE}
            --workdir ${CMAKE_CURRENT_BINARY_DIR}/work)
    set_tests_properties(mpi_analysis_${case} PROPERTIES
        LABELS "mpi;regression"
        TIMEOUT 600
        RESOURCE_LOCK mpi_analysis)
endforeach()
