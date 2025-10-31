# Auto-generated CMakeLists.txt for us_mwl_species_sim

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_mwl_species_sim
    ${src_dir}/us_mwl_species_sim.cpp
)

target_link_libraries(us_mwl_species_sim
    PRIVATE
        us_program_common
)

target_include_directories(us_mwl_species_sim
    PRIVATE
        ${src_dir}
)
