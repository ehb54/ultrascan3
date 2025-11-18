# programs/us_buoyancy/CMakeLists.txt

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_buoyancy
        ${src_dir}/us_buoyancy_main.cpp
        ${src_dir}/us_buoyancy.cpp
        )

target_link_libraries(us_buoyancy
        PRIVATE
        us_program_common
        )

target_include_directories(us_buoyancy
        PRIVATE
        ${src_dir}
        )
