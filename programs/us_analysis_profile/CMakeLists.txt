# programs/us_analysis_profile/CMakeLists.txt

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

# Everything but main(); other programs link these objects instead of
# compiling the sources again.
add_library(us_analysis_profile_objects OBJECT
        ${src_dir}/us_analysis_profile.cpp
        ${src_dir}/us_anapro_utils.cpp
        )

# pull in all the common Qt/us stuff (this is defined in programs/CMakeLists.txt)
target_link_libraries(us_analysis_profile_objects
        PUBLIC
        us_program_common
        )

target_include_directories(us_analysis_profile_objects
        PUBLIC
        ${src_dir}
        )

add_executable(us_analysis_profile
        ${src_dir}/us_anapro_main.cpp
        )

target_link_libraries(us_analysis_profile
        PRIVATE
        us_program_common
        us_analysis_profile_objects
        )
