add_executable(us_abde WIN32 MACOSX_BUNDLE
    ../us_buoyancy/us_buoyancy.cpp ../us_buoyancy/us_buoyancy.h
    us_abde_main.cpp us_abde_main.h
    us_norm_profile.cpp us_norm_profile.h
)
target_link_libraries(us_abde PUBLIC us_common)

install(TARGETS us_abde
        FRAMEWORK DESTINATION   ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
        LIBRARY DESTINATION ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
        RUNTIME DESTINATION   ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)
