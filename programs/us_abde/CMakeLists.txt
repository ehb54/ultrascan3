# programs/us_abde/CMakeLists.txt

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

# The window shared with the analysis and reporting programs, which link these
# objects instead of compiling the source again.
add_library(us_abde_objects OBJECT
        ${src_dir}/us_norm_profile.cpp
        )

target_link_libraries(us_abde_objects
        PUBLIC
        us_program_common
        )

target_include_directories(us_abde_objects
        PUBLIC
        ${src_dir}
        )

add_executable(us_abde
        ${src_dir}/us_abde_main.cpp
        )

target_link_libraries(us_abde
        PRIVATE
        us_program_common
        us_abde_objects
        us_buoyancy_objects
        )
