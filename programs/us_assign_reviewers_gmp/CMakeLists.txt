add_executable(us_assign_reviewers_gmp WIN32 MACOSX_BUNDLE
    ../us_esigner_gmp/us_esigner_gmp.cpp ../us_esigner_gmp/us_esigner_gmp.h
    us_assign_reviewers_gmp_main.cpp
)
target_link_libraries(us_assign_reviewers_gmp PUBLIC us_common)

install(TARGETS us_assign_reviewers_gmp
        FRAMEWORK DESTINATION   ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
        LIBRARY DESTINATION ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
        RUNTIME DESTINATION   ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)
