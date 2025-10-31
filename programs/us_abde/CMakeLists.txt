# programs/us_abde/CMakeLists.txt

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_abde
        ${src_dir}/us_abde_main.cpp
        ${src_dir}/us_norm_profile.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_buoyancy/us_buoyancy.cpp
        )

target_link_libraries(us_abde
        PRIVATE
        us_program_common
        )

target_include_directories(us_abde
        PRIVATE
        ${src_dir}
        ${src_dir}/../us_buoyancy
        )
