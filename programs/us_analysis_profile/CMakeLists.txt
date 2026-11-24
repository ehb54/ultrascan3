# programs/us_analysis_profile/CMakeLists.txt

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

# Everything but main(), which other programs use too, goes into us_programs_analysis.
add_library(us_analysis_profile_lib STATIC
        ${src_dir}/us_analysis_profile.cpp
        ${src_dir}/us_anapro_utils.cpp
        )

# pull in all the common Qt/us stuff (this is defined in programs/CMakeLists.txt)
target_link_libraries(us_analysis_profile_lib
        PUBLIC
        us_program_common
        )

target_include_directories(us_analysis_profile_lib
        PUBLIC
        ${src_dir}
        )

target_link_libraries(us_programs_analysis INTERFACE us_analysis_profile_lib)

add_executable(us_analysis_profile
        ${src_dir}/us_anapro_main.cpp
        )

target_link_libraries(us_analysis_profile
        PRIVATE
        us_program_common
        us_programs_analysis
        )
