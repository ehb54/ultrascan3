# programs/us_buoyancy/CMakeLists.txt

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

# Everything but main(); other programs link these objects instead of
# compiling the sources again.
add_library(us_buoyancy_objects OBJECT
        ${src_dir}/us_buoyancy.cpp
        )

target_link_libraries(us_buoyancy_objects
        PUBLIC
        us_program_common
        )

target_include_directories(us_buoyancy_objects
        PUBLIC
        ${src_dir}
        )

add_executable(us_buoyancy
        ${src_dir}/us_buoyancy_main.cpp
        )

target_link_libraries(us_buoyancy
        PRIVATE
        us_program_common
        us_buoyancy_objects
        )
