# Auto-generated CMakeLists.txt for us_predict1

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_predict1
    ${src_dir}/us_predict1_main.cpp
)

target_link_libraries(us_predict1
    PRIVATE
        us_program_common
)

target_include_directories(us_predict1
    PRIVATE
        ${src_dir}
)
