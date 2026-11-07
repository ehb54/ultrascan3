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

# Unit tests of the job XML reader (no MPI)
set(US_MPI_SRC_DIR ${CMAKE_SOURCE_DIR}/programs/us_mpi_analysis)
add_executable(test_us_mpi_jobconfig
    test_us_mpi_jobconfig.cpp
    ${US_MPI_SRC_DIR}/us_mpi_jobconfig.cpp)
target_include_directories(test_us_mpi_jobconfig PRIVATE ${US_MPI_SRC_DIR})
target_compile_definitions(test_us_mpi_jobconfig PRIVATE
    US_MPI_TEST_JOBS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/jobs")
target_link_libraries(test_us_mpi_jobconfig PRIVATE
    ${US_MPI_TEST_UTILS} Qt::Core Qt::Xml gtest gtest_main)

# Unit tests of the job's memory limit and use (no MPI)
add_executable(test_us_mpi_memlimit
    test_us_mpi_memlimit.cpp
    ${US_MPI_SRC_DIR}/us_mpi_memlimit.cpp)
target_include_directories(test_us_mpi_memlimit PRIVATE ${US_MPI_SRC_DIR})
target_link_libraries(test_us_mpi_memlimit PRIVATE Qt::Core gtest gtest_main)

# Checks job files without MPI: us_mpi_jobcheck [-q] <hpcrequest.xml>...
add_executable(us_mpi_jobcheck
    us_mpi_jobcheck.cpp
    ${US_MPI_SRC_DIR}/us_mpi_jobconfig.cpp)
target_include_directories(us_mpi_jobcheck PRIVATE ${US_MPI_SRC_DIR})
target_link_libraries(us_mpi_jobcheck PRIVATE ${US_MPI_TEST_UTILS} Qt::Core Qt::Xml)

include(GoogleTest)
gtest_discover_tests(test_us_mpi_jobconfig
    PROPERTIES
    LABELS gtest-unit-mpi_analysis
    TIMEOUT 60)
gtest_discover_tests(test_us_mpi_memlimit
    PROPERTIES
    LABELS gtest-unit-mpi_analysis
    TIMEOUT 60)

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
    2dsa_timestate
    2dsa_composite_timestate
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
    ga_np2
    dmga_basic
)

# Input-handling cases: defective or unusual job input (see INPUT_CASES)
set(US_MPI_INPUT_CASES
    input_hpcxml_missing
    input_hpcxml_two
    input_hpcxml_named
    input_hpcxml_unreadable
    input_walltime_jobxml_without
    input_walltime_cmdline_precedence
    input_ga_mutation_defaults
    input_dmga_sigma_default
    input_ga_negative_bucket
    input_rotor_stretch_short
    input_xml_truncated
    input_jobxml_truncated
    input_timestate_missing
    input_master_abort
    input_single_rank
    input_load_fails_on_worker
    input_invalid_number
    input_missing_required
    input_unknown_method
    input_meniscus_points_negative
    input_bucket_inverted
    input_unknown_key
    input_centerpiece_bottom_default
    feature_cache_off
    feature_cache_default_policy
    feature_cache_large
    feature_cache_small
    feature_cache_reused
    feature_normal
    feature_normal_check
    feature_noise_qr
    feature_normal_noise
    feature_grid_step
    feature_grid_step_one_axis
    feature_hard_misplaced
    feature_soft_walls
    feature_soft_gaussian
    feature_soft_no_margin
    feature_soft_margin
    feature_soft_prior_strong
    feature_early_stop
    feature_seed0_demes
    feature_mc_sigma_model
    feature_mc_pmasters
    feature_cache_job_limit
    feature_no_early_stop
    feature_invalid_soft
    feature_invalid_step
)

foreach(case ${US_MPI_CASES} ${US_MPI_INPUT_CASES})
    if(case IN_LIST US_MPI_INPUT_CASES)
        set(case_label input)
    else()
        set(case_label fit)
    endif()
    add_test(NAME mpi_analysis_${case}
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/us_mpi_regression.py
            check ${case}
            --program $<TARGET_FILE:us_mpi_analysis>
            --testdata $<TARGET_FILE:us_mpi_testdata>
            --mpirun ${MPIEXEC_EXECUTABLE}
            --workdir ${CMAKE_CURRENT_BINARY_DIR}/work)
    set_tests_properties(mpi_analysis_${case} PROPERTIES
        LABELS "mpi;regression;${case_label}"
        TIMEOUT 600
        RESOURCE_LOCK mpi_analysis)
endforeach()

# The node-shared simulation cache must not change results:  repeat cases
# with the cache off, with a cache too small for the fit (2DSA and PCSA keep
# the first simulations and store no more; GA keeps the recent ones and
# empties a generation of the cache again and again), with the recent policy
# for every method, with the ranks split into two pseudo-nodes (separate
# caches; the master has the other node clear its cache), with a job limit too
# small for any cache, and with the cache emptied over and over while the ranks
# use it (as under memory pressure).
set(US_MPI_CACHE_CASES
    2dsa_refine_np4
    2dsa_mc_pmasters3
    2dsa_menbot
    2dsa_composite_menbot
    2dsa_composite_menbot_pmasters
    pcsa_sl
    ga_np2
    ga_basic
)

foreach(variant "off;SimCache=off" "small;SimCacheMB=1" "recent;SimCacheMB=1,SimCachePolicy=recent"
                "nodes;NodeSplit=2" "limit;SimCacheLimitMB=1" "stress;SimCacheStress=1")
    list(GET variant 0 tag)
    list(GET variant 1 dtext)
    add_test(NAME mpi_analysis_simcache_${tag}
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/us_mpi_regression.py
            check ${US_MPI_CACHE_CASES}
            --debug-text ${dtext}
            --program $<TARGET_FILE:us_mpi_analysis>
            --testdata $<TARGET_FILE:us_mpi_testdata>
            --mpirun ${MPIEXEC_EXECUTABLE}
            --workdir ${CMAKE_CURRENT_BINARY_DIR}/work_simcache_${tag})
    set_tests_properties(mpi_analysis_simcache_${tag} PROPERTIES
        LABELS "mpi;regression;simcache"
        TIMEOUT 600
        RESOURCE_LOCK mpi_analysis)
endforeach()
