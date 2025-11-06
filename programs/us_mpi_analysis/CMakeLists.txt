# Auto-generated CMakeLists.txt for us_mpi_analysis

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_mpi_analysis
    ${src_dir}/dmga_worker.cpp
    ${src_dir}/2dsa_worker.cpp
    ${src_dir}/pmasters_compjob.cpp
    ${src_dir}/us_mpi_parse.cpp
    ${src_dir}/ga_worker.cpp
    ${src_dir}/pcsa_master.cpp
    ${src_dir}/parallel_masters.cpp
    ${src_dir}/pcsa_worker.cpp
    ${src_dir}/2dsa_master.cpp
    ${src_dir}/dmga_master.cpp
    ${src_dir}/us_mpi_analysis.cpp
    ${src_dir}/ga_master.cpp
)

target_link_libraries(us_mpi_analysis
    PRIVATE
        us_program_common
)

target_include_directories(us_mpi_analysis
    PRIVATE
        ${src_dir}
)
