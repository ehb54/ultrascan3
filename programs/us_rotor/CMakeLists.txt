# Auto-generated CMakeLists.txt for us_rotor

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_rotor
    ${src_dir}/us_rotor_main.cpp
)

target_link_libraries(us_rotor
    PRIVATE
        us_program_common
)

target_include_directories(us_rotor
    PRIVATE
        ${src_dir}
)
