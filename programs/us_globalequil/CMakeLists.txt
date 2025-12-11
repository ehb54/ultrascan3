# Auto-generated CMakeLists.txt for us_globalequil

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_globalequil
    ${src_dir}/us_fit_worker.cpp
    ${src_dir}/us_model_select.cpp
    ${src_dir}/us_globalequil.cpp
    ${src_dir}/us_eqmath.cpp
    ${src_dir}/us_long_messagebox.cpp
    ${src_dir}/us_eqfit_control.cpp
    ${src_dir}/us_eqhistogram.cpp
    ${src_dir}/us_model_adpars.cpp
    ${src_dir}/us_eqmodel_control.cpp
    ${src_dir}/us_eqreporter.cpp
)

target_link_libraries(us_globalequil
    PRIVATE
        us_program_common
)

target_include_directories(us_globalequil
    PRIVATE
        ${src_dir}
)
