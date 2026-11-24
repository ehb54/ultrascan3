# us_mwl_species_sim

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

# Everything but main(); other programs link these objects instead of
# compiling the sources again.
add_library(us_mwl_species_sim_objects OBJECT
    ${src_dir}/us_mwl_species_sim.cpp
    ${src_dir}/us_mwl_species_sim.h
)

target_link_libraries(us_mwl_species_sim_objects
    PUBLIC
        us_program_common
)

target_include_directories(us_mwl_species_sim_objects
    PUBLIC
        ${src_dir}
)

add_executable(us_mwl_species_sim
    ${src_dir}/us_mwl_species_sim_main.cpp
)

target_link_libraries(us_mwl_species_sim
    PRIVATE
        us_program_common
        us_mwl_species_sim_objects
)
