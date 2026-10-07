# Auto-generated CMakeLists.txt for us_mwl_species_sim

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_mwl_species_sim
    ${src_dir}/us_mwl_species_sim_main.cpp
    ${src_dir}/us_mwl_species_sim.cpp
    ${src_dir}/us_mwl_species_sim.h
)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Xml)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Xml)

target_link_libraries(us_mwl_species_sim
    PRIVATE
        us_program_common
        Qt${QT_VERSION_MAJOR}::Xml
)

target_include_directories(us_mwl_species_sim
    PRIVATE
        ${src_dir}
)
