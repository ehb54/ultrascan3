add_executable(us_audit_trail_gmp WIN32 MACOSX_BUNDLE
    us_audit_trail_gmp.cpp us_audit_trail_gmp.h
    us_audit_trail_gmp_main.cpp
)
target_link_libraries(us_audit_trail_gmp PUBLIC us_common)

install(TARGETS us_audit_trail_gmp
        FRAMEWORK DESTINATION   ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
        LIBRARY DESTINATION ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
        RUNTIME DESTINATION   ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)