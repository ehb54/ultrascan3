# Auto-generated CMakeLists.txt for us_mwl_spectra

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_mwl_spectra
    ${src_dir}/us_mwls_pltctl.cpp
    ${src_dir}/us_mwl_spectra.cpp
)

target_link_libraries(us_mwl_spectra
    PRIVATE
        us_program_common
)

target_include_directories(us_mwl_spectra
    PRIVATE
        ${src_dir}
)
