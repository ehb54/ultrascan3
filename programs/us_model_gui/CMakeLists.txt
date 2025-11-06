# Auto-generated CMakeLists.txt for us_model_gui

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_model_gui
    ${src_dir}/main.cpp
)

target_link_libraries(us_model_gui
    PRIVATE
        us_program_common
)

target_include_directories(us_model_gui
    PRIVATE
        ${src_dir}
)
