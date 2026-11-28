# us_mwl_species_sim

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

# Everything but main(), which other programs use too, goes into us_programs_analysis.
add_library(us_mwl_species_sim_lib STATIC
    ${src_dir}/us_mwl_species_sim.cpp
    ${src_dir}/us_mwl_species_sim.h
)

target_link_libraries(us_mwl_species_sim_lib
    PUBLIC
        us_program_common
)

target_include_directories(us_mwl_species_sim_lib
    PUBLIC
        ${src_dir}
)

target_link_libraries(us_programs_analysis INTERFACE us_mwl_species_sim_lib)

add_executable(us_mwl_species_sim
    ${src_dir}/us_mwl_species_sim_main.cpp
)

target_link_libraries(us_mwl_species_sim
    PRIVATE
        us_program_common
        us_programs_analysis
)
