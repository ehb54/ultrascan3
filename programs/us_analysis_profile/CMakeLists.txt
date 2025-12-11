# programs/us_analysis_profile/CMakeLists.txt

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

# List every source this program actually uses
set(US_ANALYSIS_PROFILE_SOURCES
        ${src_dir}/us_analysis_profile.cpp
        ${src_dir}/us_anapro_main.cpp
        ${src_dir}/us_anapro_utils.cpp
        )

add_executable(us_analysis_profile
        ${US_ANALYSIS_PROFILE_SOURCES}
        )

# pull in all the common Qt/us stuff (this is defined in programs/CMakeLists.txt)
target_link_libraries(us_analysis_profile
        PRIVATE
        us_program_common
        )

target_include_directories(us_analysis_profile
        PRIVATE
        ${src_dir}
        )
