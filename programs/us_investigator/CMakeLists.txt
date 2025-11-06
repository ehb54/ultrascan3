# Auto-generated CMakeLists.txt for us_investigator

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(us_investigator
    ${src_dir}/us_investigator_main.cpp
)

target_link_libraries(us_investigator
    PRIVATE
        us_program_common
)

target_include_directories(us_investigator
    PRIVATE
        ${src_dir}
)
