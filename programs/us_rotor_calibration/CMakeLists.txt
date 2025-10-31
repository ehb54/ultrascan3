# Auto-generated CMakeLists.txt for us_rotor_calibration

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_rotor_calibration
    ${src_dir}/us_rotor_calibration.cpp
    ${src_dir}/us_get_dbexp.cpp
)

target_link_libraries(us_rotor_calibration
    PRIVATE
        us_program_common
)

target_include_directories(us_rotor_calibration
    PRIVATE
        ${src_dir}
)
