# Auto-generated CMakeLists.txt for us_spectrum

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_spectrum
    ${src_dir}/us_spectrum.cpp
)

target_link_libraries(us_spectrum
    PRIVATE
        us_program_common
)

target_include_directories(us_spectrum
    PRIVATE
        ${src_dir}
)
