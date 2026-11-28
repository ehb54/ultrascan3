# programs/us_buoyancy/CMakeLists.txt

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

# Everything but main(), which other programs use too, goes into us_programs_analysis.
add_library(us_buoyancy_lib STATIC
        ${src_dir}/us_buoyancy.cpp
        )

target_link_libraries(us_buoyancy_lib
        PUBLIC
        us_program_common
        )

target_include_directories(us_buoyancy_lib
        PUBLIC
        ${src_dir}
        )

target_link_libraries(us_programs_analysis INTERFACE us_buoyancy_lib)

add_executable(us_buoyancy
        ${src_dir}/us_buoyancy_main.cpp
        )

target_link_libraries(us_buoyancy
        PRIVATE
        us_program_common
        us_programs_analysis
        )
