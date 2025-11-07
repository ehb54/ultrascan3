# Auto-generated CMakeLists.txt for us_astfem_sim

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_astfem_sim
    ${src_dir}/us_astfem_sim.cpp
    ${src_dir}/us_clipdata.cpp
)

target_link_libraries(us_astfem_sim
    PRIVATE
        us_program_common
)

target_include_directories(us_astfem_sim
    PRIVATE
        ${src_dir}
)
