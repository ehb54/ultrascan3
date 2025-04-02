add_executable(us_analysis_profile WIN32 MACOSX_BUNDLE
    us_analysis_profile.cpp us_analysis_profile.h
    us_anapro_main.cpp
    us_anapro_utils.cpp
)
target_link_libraries(us_analysis_profile PUBLIC us_common)

install(TARGETS us_analysis_profile
        FRAMEWORK DESTINATION   ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
        LIBRARY DESTINATION ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
        RUNTIME DESTINATION   ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)
